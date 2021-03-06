
#include "CWVehicle.h"
#include "CarWorld.h"
#include "H_Object.h"
#include "H_Graphics.h"
#include "H_Variable.h"
#include "H_XML.h"
#include <boost/bind.hpp>
#include "CWBeeper.h"
#include "CarWorldClient.h"
#include "boost/foreach.hpp"
#include "OFFObjectPool.h"

extern ofstream herr;
extern CarWorldClient* pCWC;

CWCommand::CWCommand() : GasBrake(0), Steer(0), SteerFeedBack(0), HandBrake(false)
{}

//CLASS Wheel:
Wheel::Wheel(
	const Point3D &RelPos,
	REAL SpringStiffness,
	REAL MaxTravel,
	REAL MaxLoad,
	REAL WheelRadius,
	REAL DamperCompression,
	REAL DamperRebound,
	REAL Grip,
	REAL AccelFactor,
	REAL BrakeFactor,
	REAL SteerFactor,
	bool LockUp
	) :
RelPos(RelPos),
	SpringStiffness(SpringStiffness),
	MaxTravel(MaxTravel),
	MaxLoad(MaxLoad),
	WheelRadius(WheelRadius),
	DamperCompression(DamperCompression),
	DamperRebound(DamperRebound),
	Grip(Grip),
	AccelFactor(AccelFactor),
	BrakeFactor(BrakeFactor),
	SteerFactor(SteerFactor),
	LockUp(LockUp)
{
	reset();
}

void Wheel::draw_init()
{
	Model.readfile(ModelFile.c_str());
	Model.Scale(ModelScale);
	Model.Translate(ModelPos);
}

void Wheel::reset()
{
	Acceleration = Point3D(0,0,0);
	Angle = 0;
	DeltaTravel = 0;
	Travel = 0;
	Gs = 0;
	LandDirection = Point3D(0,0,0);
	LandSpeed = Point3D(0,0,0);
	Lock = false;
	Thrust = 0;
}
void Wheel::Set(const Ref &ARef, CWCommand &ACommand, CarWorld &CW)
{
	MyRef = ARef;

	//dissociate Gas from Break
	REAL Gas = LIMIT(0.f,ACommand.GasBrake,1.f);
	REAL Brake = LIMIT(0.f,-ACommand.GasBrake,1.f);
	Lock = ACommand.HandBrake;

	MyRef.Rotate(MyRef.GetUp()*(ACommand.Steer*SteerFactor));

	if (MyRef.Speed*MyRef.GetDirection()>0)
		Thrust = (Gas*AccelFactor - Brake*BrakeFactor);
	else //the gas pedal switches with the Brake
		Thrust = (Gas*BrakeFactor - Brake*AccelFactor);

	//Hit the ground surface
	MyContact = CW.m_Landscape->GetContact(GetRay());

	if (MyContact.Found)
	{
		Point3D ToGround = MyContact.Position-MyRef.Position;
		REAL OldTravel = Travel;
		Travel = LIMIT(0,MyRef.GetUp()*ToGround,-MaxTravel);
		MyRef.Position += MyRef.GetUp()*(Travel);
		DeltaTravel = (Travel-OldTravel)/Ref::TimeIncrement;

		//calculate the LandSpeed and LandDirection:
		if (Travel>0)
		{
			LandSpeed =
				MyContact.Normal^(MyRef.Speed^MyContact.Normal);
			//calc LandDirection:
			Point3D W = MyRef.GetUp()^MyRef.GetDirection();
			LandDirection = W^MyContact.Normal;
			LandDirection.normalize();
			Angle += (LandSpeed*LandDirection*Ref::TimeIncrement)/(2*WheelRadius);
		}
	}
	else
	{
		Travel = 0;
		LandSpeed.reset();
		LandDirection.reset();
	}
}

Point3D Wheel::CalcForce()
{
	Point3D Force(0,0,0);
	if (MyContact.Found)
	{
		REAL Load;
		//only apply force if the susp is under load:
		if (DeltaTravel>0)
			Load = (SpringStiffness*Travel
			+DamperCompression*DeltaTravel);
		else //DeltaTravel<0
			Load = (SpringStiffness*Travel
			+DamperRebound*DeltaTravel);

		if (Load>0)
		{
			//tanO = sinO/cosO
			REAL tanO = ((MyContact.Normal^(MyRef.GetUp())).norm())
				/(MyContact.Normal*MyRef.GetUp());
			tanO = LIMIT(0.f,tanO,1.f);
			Load = (1+tanO)*Load;
			Force = MyContact.Normal * Load;
		}
		else
			Force.reset();

		Load = H_MAX(0,Load);

		//calculate where the wheel is pushing:
		//laterally
		Point3D Traction = (LandDirection*(LandSpeed*LandDirection)-LandSpeed)*(Load);
		//ForceFeedback = Traction.norm()/
		//Longitudinally:
		Traction += LandDirection*Thrust;
		if (LockUp && Lock)
			Traction = -LandSpeed*Load;
		//try too mutch traction and you slip:
		//max traction depends on load and grip
		//but the load is limited to MaxLoad
		//to avoid wierd reactions
		//ex: at jump receptions
		REAL MaxTraction = min(Load*Grip,MaxLoad*Grip);
		if (Traction.norm()>MaxTraction)
			Traction.normalize(MaxTraction);
		Force += Traction;
	}
	else
		Force.reset();
	return Force;
}
FixedVector &Wheel::GetRay()
{
	Ray.Position = MyRef.Position + MyRef.GetUp()*(-2*MaxTravel);
	Ray.Value = MyRef.GetUp();
	Ray.Value = -Ray.Value;
	return Ray;
}

void Wheel::draw()
{
	//Move to the wheel location an draw it
	Ref RenderRef = MyRef;
	RenderRef.Position += RenderRef.Z*WheelRadius;
	RenderRef.Rotate(RenderRef.X*(-Angle));
	glEnable(GL_LIGHTING);
	Model.draw(RenderRef);
}

//CLASS CWVehicle:
CWVehicle::CWVehicle(const char *name)
	: m_bShowBox(false)
{
	MyDriveTrain = DriveTrain(RedLine, MaxTorque);
	MyRef = InertRef(Mass, MassDistrib);

	load(name);
	elapsed_time=0;
	distractorN=0;
	distractor.clear();
	collisionTime=-10000.0; // so that the first collision alway happen

	bFakeCar=false;
	vector<string> _tags=OFFObjectPool::sharedOFFPool()->getTags();
	BOOST_FOREACH(const string&tag,_tags)
	{
		m_HitCount[tag]=0;
	}
}

void CWVehicle::load(const char *name)
{
	ifstream in;
	OpenDataFile(in,name);

	map<string,HVariable*> DescribeVhc;
	DescribeVhc["ModelFile"] =		new HVar<string>("ModelFile",&ModelFile);
	DescribeVhc["ModelPos"] =			new HVar<Point3D>("ModelPos",&ModelPos);
	DescribeVhc["ModelScale"] =			new HVar<Point3D>("ModelScale",&ModelScale);

	DescribeVhc["InitPos"] =		new HVar<Point3D>("InitPos",&InitPos);
	DescribeVhc["Mass"] =			new HVar<REAL>("Mass",&Mass);
	DescribeVhc["MassDistrib"] =	new HVar<REAL>("MassDistrib",&MassDistrib);
	DescribeVhc["RedLine"] =		new HVar<REAL>("RedLine",&RedLine);
	DescribeVhc["MaxTorque"] =		new HVar<REAL>("MaxTorque",&MaxTorque);
	DescribeVhc["Friction"] =		new HVar<REAL>("Friction",&Friction);

	Wheel W;
	map<string,HVariable*> DescribeWheel;
	DescribeWheel["ModelFile"] =				new HVar<string>("ModelFile",&W.ModelFile);
	DescribeWheel["ModelPos"] =					new HVar<Point3D>("ModelPos",&W.ModelPos);
	DescribeWheel["ModelScale"] =				new HVar<Point3D>("ModelScale",&W.ModelScale);

	DescribeWheel["RelPos"] =				new HVar<Point3D>("RelPos",&(W.RelPos));
	DescribeWheel["SpringStiffness"] =		new HVar<REAL>("SpringStiffness",&(W.SpringStiffness));
	DescribeWheel["MaxTravel"] =			new HVar<REAL>("MaxTravel",&(W.MaxTravel));
	DescribeWheel["MaxLoad"] =				new HVar<REAL>("MaxLoad",&(W.MaxLoad));
	DescribeWheel["WheelRadius"] =			new HVar<REAL>("WheelRadius",&(W.WheelRadius));
	DescribeWheel["DamperCompression"] =	new HVar<REAL>("DamperCompression",&(W.DamperCompression));
	DescribeWheel["DamperRebound"] =		new HVar<REAL>("DamperRebound",&(W.DamperRebound));
	DescribeWheel["Grip"] =					new HVar<REAL>("Grip",&(W.Grip));
	DescribeWheel["AccelFactor"] =			new HVar<REAL>("AccelFactor",&(W.AccelFactor));
	DescribeWheel["BrakeFactor"] =			new HVar<REAL>("BrakeFactor",&(W.BrakeFactor));
	DescribeWheel["SteerFactor"] =			new HVar<REAL>("SteerFactor",&(W.SteerFactor));
	DescribeWheel["LockUp"] =				new HVar<bool>("LockUp",&(W.LockUp));

	CWBeeper BP;
	map<string,HVariable*> DescribeBeeper;
	DescribeBeeper["BeeperName"] =			new HVar<string>("BeeperName",&(BP.BeeperName));
	DescribeBeeper["SoundFile"] =			new HVar<string>("SoundFile",&(BP.Filename));
	DescribeBeeper["DefaultKeyBind"] =			new HVar<string>("DefaultKeyBind",&(BP.Keybinding));


	XmlTag tag;
	do
	{
		tag.read_from(in);
		switch (tag.get_type())
		{
		case XmlTag::broken:
			throw HException("parse error in vehicle file");
		case XmlTag::opening:
			{
				if (tag.name() == "CWVehicle")
				{
					tag.write_to(DescribeVhc);
					Weight = Mass*EARTH_GRAVITY;
				}
				if (tag.name() == "CWWheel")
				{
					W = Wheel();
					tag.write_to(DescribeWheel);
					Wheels.push_back(W);
				}
				if (tag.name()== "CWBeeper")
				{
					BP = CWBeeper();
					tag.write_to(DescribeBeeper);
					Beepers.push_back(BP);
				}
			}
		default:;
		}
	} while ((tag.get_type() != XmlTag::broken) && (tag.get_type() != XmlTag::empty));

	map<string,HVariable*>::iterator I;
	for (I = DescribeVhc.begin() ; I!=DescribeVhc.end() ; I++)
		delete (*I).second;
	for (I = DescribeWheel.begin() ; I!=DescribeWheel.end() ; I++)
		delete (*I).second;

	MyDriveTrain = DriveTrain(RedLine, MaxTorque);
	MyRef = InertRef(Mass, MassDistrib);
}

CWVehicle::~CWVehicle()
{}

void CWVehicle::reset()
{
	//cout << "in car.reset()\n";
	MyRef.reset();
	MyRef.Position = InitPos;
	MyCommand = CWCommand();

	for (vector<Wheel>::iterator I = Wheels.begin() ; I != Wheels.end() ; I++)
		(*I).reset();

	//cout << "updating car params...\n";
	UpdateCWVehicleParams();
	//cout << "finnished resetting car...\n";
}

void CWVehicle::update()
{
	if (is_vehicle_out_of_road())
	{
		reset_to_fall_block();
		return;
	}

	static Point3D Gravity(0,0,-EARTH_GRAVITY);

	//Add the suspention forces:
	MyCommand.SteerFeedBack = 0;
	for (vector<Wheel>::iterator I = Wheels.begin() ; I != Wheels.end() ; I++)
	{
		Point3D Force = (*I).CalcForce();
		MyRef.Apply(FixedVector((*I).MyRef.Position, Force));
		//plug force feedback on lateral acceleration of the steering wheels
		if ((*I).SteerFactor!=0)
		{
			MyCommand.SteerFeedBack += (MyRef.GetX()*Force)/(MyRef.GetMass()*Wheels.size());
		}
	}

	//gravity
	MyRef.Apply(FixedVector(MyRef.Position, Gravity*MyRef.GetMass()));
	//friction
	MyRef.Apply(FixedVector(MyRef.Position, MyRef.Speed*(-Friction*MyRef.Speed.norm())));

	MyRef.TimeClick();

	//make the rest of the car follow:
	UpdateCWVehicleParams();

	//     Model.MyBox.VisitAllPoint(boost::bind(&CWVehicle::LocalPosToGlobalPos,this,_1));
	CollisionTest();
}

REAL CWVehicle::GetSpeed() const //returns speed in kph
{
	return MyRef.Speed.norm()*3.6f; // 3.6 = 3600/1000 to get kph
}

void CWVehicle::UpdateCWVehicleParams()
{
	for (vector<Wheel>::iterator I = Wheels.begin() ; I != Wheels.end() ; I++)
		(*I).Set(
		MyRef.GetRef((*I).RelPos),
		MyCommand,
		*m_CarWorld
		);
}

void CWVehicle::draw_init()
{
	Model.readfile(ModelFile.c_str());
	//Model.Center();
	Model.Scale(ModelScale);
	Model.Translate(ModelPos);
	//Model.SetShading(false);
	for (vector<Wheel>::iterator I = Wheels.begin() ; I != Wheels.end() ; I++)
	{
		(*I).draw_init();
	}

	for(vector<CWBeeper>::iterator I = Beepers.begin(); I!= Beepers.end(); ++I)
	{
		I->init();
	}
}

void CWVehicle::draw()
{
	if (Hgl::GetShadows()>0&&!bFakeCar)
		ProjectShadow(m_CarWorld->LightDirection);
	drawShape();
}

void CWVehicle::drawShape()
{
	//draw the Wheels:
	for (vector<Wheel>::iterator I = Wheels.begin() ; I != Wheels.end() ; I++)
		(*I).draw();

	if(bFakeCar)
	{
		Ref ref = MyRef.GetRef(Point3D(0,0,0));
		glPushMatrix();
		Hgl::Relocate(ref);
		glDisable(GL_LIGHTING);
		glColor3f(.0f,.0f,.0f);
		Model.MyBox.DrawFrame();
		glEnable(GL_LIGHTING);
		glPopMatrix();
		return;
	}

	//don't draw the body if we are using this car's interior-camera
	InCarCam* InCam = dynamic_cast<InCarCam*>(m_CarWorld->m_Camera);
	if ((InCam==NULL) || (InCam->m_Vehicle!=this))
		Model.draw(MyRef.GetRef(Point3D(0,0,0)),m_bShowBox);
}

void CWVehicle::ProjectShadow(const Point3D &LightDirection)
{
	//static Point3D ShadowPlane[3] = {Point3D(0,0,0),Point3D(1,0,0),Point3D(0,1,0)};

	Contact ShadowContact =
		m_CarWorld->m_Landscape->GetContact(FixedVector(MyRef.Position,-LightDirection));

	if (ShadowContact.Found)
	{
		Hgl::Enable(Hgl::SHADOW);
		glPushMatrix();
		Hgl::ShadowTransform(LightDirection,ShadowContact);
		drawShape();
		glPopMatrix();
		Hgl::Disable(Hgl::SHADOW);
	}
}

void CWVehicle::drawInfo()
{
	MyDriveTrain.drawInfo(MyRef.Speed.norm());

}

CWVehicleState CWVehicle::GetState()
{
	CWVehicleState tmp;
	tmp.m_Ref = MyRef;
	tmp.m_Command = MyCommand;
	return tmp;
}

void CWVehicle::SetState(CWVehicleState &state)
{
	*(Ref*)(&MyRef) = state.m_Ref;
	MyCommand = state.m_Command;
}

bool CWVehicle::is_vehicle_out_of_road()
{
	for(vector<Wheel>::const_iterator it = Wheels.begin();it!=Wheels.end();++it)
	{
		Contact tmp = m_CarWorld->m_Landscape->GetPointContact(it->MyRef.Position,4.0);
		if(tmp.Found)
			return false;
	}
	AudioPlayer::shared_audio()->get_sound("Fall")->play_once();
	nirs.on(elapsed_time);
	CCrashRec temp;
	temp.time=elapsed_time;
	temp.type=0;
	Crashrecord.push_back(temp);
	return true;
}

void CWVehicle::reset_to_fall_block()
{
	CWLandscape& landscape = *(m_CarWorld->m_Landscape);
	if(landscape.LastContactBlock!=landscape.MyWorldBlocks.end())
	{

		WorldBlock::MyTriangle* LastHitTri = landscape.LastContactTriangle;

		Point3D reset_pt;

		if(LastHitTri!=NULL)
		{
			reset_pt = LastHitTri->GetPointByUV(0,landscape.LastV);
		}
		else
		{
			reset_pt = LastHitTri->GetPointByUV(0,0);
		}

		// notice that edge v0v2 consist the road curb

		Point3D forward_direction = LastHitTri->GetForwardDirection();
		forward_direction.normalize();
		reset();
		MyRef.Position = reset_pt;
		MyRef.Y = forward_direction;
		MyRef.Position.z()+= 2.0;
	}
	else
		reset();
}

Point3D CWVehicle::GetCenterPos() const
{
	return MyRef.GetAbsCoord(Model.MyBox.GetCenter());
}

void CWVehicle::GetBox3D( Box3D& box ) const
{
	box = Model.MyBox;
	box.VisitAllPoint(boost::bind(&CWVehicle::LocalPosToGlobalPos,this,_1));
}

bool CWVehicle::IsPointInside( const Point3D& pt ) const
{
	Box3D box;
	GetBox3D(box);
	return box.IsPtInside(pt);
}

void CWVehicle::LocalPosToGlobalPos( Point3D& pt )const
{
	pt = MyRef.GetAbsCoord(pt);
}

void CWVehicle::CollisionTest()
{
	// by LX:
	// i'm now using m_ObjIsColliding to keep collision happen only once.
	// this statement takes me 2 hour to debug this problem. 
	// if we do collision to slow, then ,if two object close enough, the second collision would not be detected.
	// so, if you can keep objects away from each other so that the car would not pass two of them in a second, then this code is ok.
// 	if ( elapsed_time-collisionTime<1) // atleast 1 second 
// 		return;
	if(bFakeCar)// we do not do colliding test when it's a fake car.
		return;

	Box3D box;
	GetBox3D(box);
	CCrashRec temp;
	static void* pRem=NULL;
	for(vector<CWPointObject*>::iterator it = m_ObjectsToCollade.begin(); it != m_ObjectsToCollade.end(); )
	{
		if((*it)->IsCollideWithBox(box))
		{
			if(!m_ObjIsColliding[(*it)])// it is not in colliding state
			{
				// execute colliding script!
				if(pCWC)
				{
					pCWC->exec_file((*it)->ScriptFile.c_str());
				}
				// record the hit count;
				++m_HitCount[(*it)->GetTag()];
				m_ObjIsColliding[(*it)]=true;
			}

			if((*it)->GetTag()=="mushroom")
			{
				nirs.on(elapsed_time); 
				collisionTime=elapsed_time;
				temp.time=elapsed_time;
				temp.type=2;
				Crashrecord.push_back(temp);

			}
			else if((*it)->GetTag()=="cone")
			{
				nirs.on(elapsed_time); 
				collisionTime=elapsed_time;
				temp.time=elapsed_time;
				temp.type=1;
				Crashrecord.push_back(temp);
			}
		}
		else
		{
			m_ObjIsColliding[(*it)]=false;
		}
		++it;
	}
}

void CWVehicle::AddToColladeList( CWPointObject* object )
{
	m_ObjectsToCollade.push_back(object);
	m_ObjIsColliding[object]=false;
}

void CWVehicle::updateTime(double t){ 
	elapsed_time=t;
	nirs.update(t);
	eeg.update(t);
	if (  distractorN >= distractor.size() || distractor[distractorN]->time>elapsed_time ) return;
	switch (distractor[distractorN]->type){
	case 0:
		//herr<<" "<<distractor[distractorN]->time<<" "<<distractor[distractorN]->str<<endl;
		AudioPlayer::shared_audio()->get_sound(distractor[distractorN++]->str)->play_once();
		//AudioPlayer::shared_audio()->get_sound("HitCone")->play_once();

		//distractorN++;
		break;
	case 1:
		return;
	}
}

void CWVehicle::AddToDistractor( const double t, const int tp, const char *c , const int dur)
{
	CDistractor *d=new CDistractor;
	d->duration=dur;
	d->str=string(c);
	d->time=t;
	d->type=tp;
	distractor.push_back(d);
}

void CWVehicle::RemoveFromeCollideList( CWPointObject* object )
{
	vector<CWPointObject*>::iterator it;
	it=std::find(m_ObjectsToCollade.begin(),m_ObjectsToCollade.end(),object);
	if(it!=m_ObjectsToCollade.end())
	{
		m_ObjectsToCollade.erase(it);
		m_ObjIsColliding.erase(m_ObjIsColliding.find(object));
	}
}

function varargout = road_editor(varargin)
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @road_editor_OpeningFcn, ...
                   'gui_OutputFcn',  @road_editor_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end

% select point by mouse in axis
function road_show_ButtonDownFcn(hObject, eventdata, handles)
global gMyData gHandles;
if gMyData.is_selecting
    curpt = get(hObject,'CurrentPoint');
    pt = curpt(2,1:2);
    trinum = get_triangle_num();
    left = gMyData.road_edge1;
    right = gMyData.road_edge2;
    tri = get_triangle_by_pos(pt);
    if tri.index == -1
        return
    end
    % because points are shared, so calculate a range
    tri.pt_range = calculate_pt_range(tri);
    % is ctrl pressed?
    modifiers = get(gcf,'currentModifier');        
    ctrlIsPressed = ismember('control',modifiers);
    if ctrlIsPressed
        len = size(gMyData.select,2);
        is_del = false;
        for i = 1:len
            if gMyData.select(i).index == tri.index;
                gMyData.select(i).index=-1;
                is_del = true;
            end
        end
        if ~is_del
            gMyData.select = [gMyData.select,tri];
        end
        gMyData.select = gMyData.select([gMyData.select.index]~=-1);
        draw_all_select();
        init_select_uicontrol();
        % enable all_sel_z control
        set(gHandles.all_sel_z,'String','','enable','on');
    else
        gMyData.select = tri;
        draw_select();
        init_set_all_z_uicontrol();
        % enable pt1/2/3_z control
        set(gHandles.pt1_z,'enable','on');
        set(gHandles.pt2_z,'enable','on');
        set(gHandles.pt3_z,'enable','on');
    end
end

function ptnum = get_line_pt_num()
global gMyData;
ptnum = size(gMyData.mid_line,1);

function trinum = get_triangle_num()
trinum = (get_line_pt_num()-1)*2;
if trinum<0
    trinum=0
end

function tri = get_triangle_by_pos(pos)
global gMyData;
left = gMyData.road_edge1;
right = gMyData.road_edge2;
for i=1:get_triangle_num()
    pt_index = get_pt_index_by_tri_indice(i);
    pts = get_pts_by_index(pt_index);
    is_inside = is_pt_inside(pos,pts(1,:),pts(2,:),pts(3,:));
    if is_inside
        tri.index = i;
        tri.pts = pts;
        tri.pt_index = pt_index;
        return
    end
end
tri.index = -1;
tri.pts=[];
tri.pt_index=[];

% pt_index contains two fields: 1,which line; 2,index number.
% this function returns all three pt index of a give triangle
function pt_index = get_pt_index_by_tri_indice(tri_indice)
i = tri_indice;
a = floor((i-1)/2);
b = mod(i-1,2);
if b == 0
    pt_index = [1,a+1;2,a+1;1,a+2]; % field1: 1 means left, 2 means right.
else
    pt_index = [1,a+2;2,a+1;2,a+2];
end

% this returns pts by index
function pts = get_pts_by_index(pt_index)
global gMyData;
left = gMyData.road_edge1;
right = gMyData.road_edge2;
for i = 1:size(pt_index,1)
    if pt_index(i,1)==1
        pts(i,:) = left(pt_index(i,2),:);
    else
        pts(i,:) = right(pt_index(i,2),:);
    end
end

% this returns pts+zvalue by index
function pts = get_ptsz_by_index(pt_index)
global gMyData;
left = gMyData.road_edge1;
leftz = gMyData.road_edge1_z;
right = gMyData.road_edge2;
rightz = gMyData.road_edge2_z;
for i = 1:size(pt_index,1)
    indice = pt_index(i,2);
    if pt_index(i,1)==1
        pts(i,:) = [left(indice,:),leftz(indice)];
    else
        pts(i,:) = [right(indice,:),rightz(indice)];
    end
end

% if multi-select
function draw_all_select()
global gMyData;
for i = 1:size(gMyData.select_h,2)
    del_old_handle(i);
end
for i = 1:size(gMyData.select,2)
    draw_sel_tri(i);
end

% delete one selete
function del_old_handle(i)
global gMyData;
if isfield(gMyData,'select_h') && i<=size(gMyData.select_h,2)
    if ishandle(gMyData.select_h(i)) 
        delete(gMyData.select_h(i));
        gMyData.select_h(i)=-1;
    end
end

% draw one select
function draw_sel_tri(i)
global gMyData;
if gMyData.select(i).index==-1
    return
end
hold on;
slt = [gMyData.select(i).pts;gMyData.select(i).pts(1,:)];
gMyData.select_h(i) = plot(slt(:,1),slt(:,2),'k-','linewidth',3);
hold off;

% draw the selected triangle and update panel
function draw_select()
global gMyData gHandles;
for i = 1:size(gMyData.select_h,2)
    del_old_handle(i);
end
% draw triangle line
draw_sel_tri(1);
% update panel
update_select_uicontrol();

% init set all z control
function init_set_all_z_uicontrol()
global gMyData gHandles;
set(gHandles.all_sel_z,'String','nil','enable','off');

% init select-pt-set control
function init_select_uicontrol()
global gMyData gHandles;
set(gHandles.pt1_x,'String','nil');
set(gHandles.pt1_y,'String','nil');
set(gHandles.pt1_z,'String','nil','enable','off');
set(gHandles.pt1_index,'String','nil');
set(gHandles.pt2_x,'String','nil');
set(gHandles.pt2_y,'String','nil');
set(gHandles.pt2_z,'String','nil','enable','off');
set(gHandles.pt2_index,'String','nil');
set(gHandles.pt3_x,'String','nil');
set(gHandles.pt3_y,'String','nil');
set(gHandles.pt3_z,'String','nil','enable','off');
set(gHandles.pt3_index,'String','nil');

function s = pt_index_str(line_no,min,max)
global gMyData;
if min==max
    s_i = sprintf('%d',min);
else
    s_i = sprintf('%d-%d',min,max);
end
if line_no == 1
   s_l = 'left';
else
   s_l = 'right';
end
s = sprintf('%s:%s',s_l,s_i);

% update select-pt-set controller
function update_select_uicontrol()
global gMyData gHandles;
if size(gMyData.select,1)~=1
    init_select_uicontrol();
    return
end
for i = 1:3
    handle_x = getfield(gHandles,sprintf('pt%d_x',i));
    handle_y = getfield(gHandles,sprintf('pt%d_y',i));
    handle_z = getfield(gHandles,sprintf('pt%d_z',i));
    handle_index=getfield(gHandles,sprintf('pt%d_index',i));

    ptz = get_ptsz_by_index(gMyData.select.pt_index(i,:));
    set(handle_x,'String',num2str(ptz(1)));
    set(handle_y,'String',num2str(ptz(2)));
    set(handle_z,'String',num2str(ptz(3)));
    
    line_no = gMyData.select.pt_index(i,1);
    range = gMyData.select.pt_range(i,:);
    set(handle_index,'String',pt_index_str(line_no,range(1),range(2)));
end
% delete last draw
if isfield(gMyData,'h_sel_pts')
    for handle = gMyData.h_sel_pts
        if ishandle(handle)
            delete(handle);
        end
    end
    gMyData.h_sel_pts = [];
end
% draw the 3 points

global X Y Z
contour(X,Y,Z);
'hi'
hold on;
pt = get_pts_by_index(gMyData.select.pt_index(1,:));
gMyData.h_sel_pts(1) = plot(pt(1),pt(2),'ro','MarkerSize',10);
pt = get_pts_by_index(gMyData.select.pt_index(2,:));
gMyData.h_sel_pts(2) = plot(pt(1),pt(2),'rx','MarkerSize',10);
pt = get_pts_by_index(gMyData.select.pt_index(3,:));
gMyData.h_sel_pts(3) = plot(pt(1),pt(2),'r*','MarkerSize',10);
hold off;

% calculate pt index range
function range = calculate_pt_range(sel)
global gMyData;
for i = 1:size(sel.pt_index,1)
    indice = sel.pt_index(i,:);
    if indice(1)==1
        line_array = gMyData.road_edge1;
    else
        line_array = gMyData.road_edge2;
    end
    % find same point backward and forward
    for j = indice(2):-1:1
        if isequal(line_array(j,:),line_array(indice(2),:))
            min = j;
        else
            break;
        end
    end
    for j = indice(2):size(line_array,1)
        if isequal(line_array(j,:),line_array(indice(2),:))
            max = j;
        else
            break;
        end
    end
    range(i,:) = [min,max];
end


% -- judge if the point is inside the triangle
function is_inside = is_pt_inside(pt,tr1,tr2,tr3)
if isequal(tr1,tr2) || isequal(tr2,tr3) || isequal(tr3,tr1)
    is_inside = false;
    return
end
v1 = [tr1-pt,0];
v2 = [tr2-pt,0];
v3 = [tr3-pt,0];
v1_v2 = cross(v1,v2);
v2_v3 = cross(v2,v3);
v3_v1 = cross(v3,v1);
r1 = dot(v1_v2,v2_v3);
r2 = dot(v2_v3,v3_v1);
if r1>=0 && r2>=0
    is_inside = true;
else
    is_inside = false;
end

% --- Executes just before road_editor is made visible.
function road_editor_OpeningFcn(hObject, eventdata, handles, varargin)
handles.output = hObject;
guidata(hObject, handles);
% set keyboard function
set(hObject,'KeyPressFcn',@MyKeyPressFcn);
set(handles.btn_export,'Callback',@ExportBlockData_callback);
% init data
clear_all_lines_data();
global gMyData gHandles;
gHandles = handles;

gMyData.is_drawing=false;
gMyData.is_selecting=false;
init_select_uicontrol();
init_set_all_z_uicontrol();

function ExportBlockData_callback(hObject,eventdata)
[FileName,PathName] = uiputfile('*.txt','Save the block data','Untitled.txt');
if FileName == 0
    return
end
global gMyData gHandles;
left_z = gMyData.road_edge1_z; right_z = gMyData.road_edge2_z;
gMyData.mid_line_z = (left_z+right_z)/2;

generate_all_lines_uv();
filename = sprintf('%s%s',PathName,FileName);
block = get_block();
write_roadblock(filename,block);

function block = get_block()
global gMyData gHandles;
block.point_num = get_line_pt_num()*2;
block.tri_num = get_triangle_num();
left = gMyData.road_edge1; right = gMyData.road_edge2; mid = gMyData.mid_line;
left_z = gMyData.road_edge1_z; right_z = gMyData.road_edge2_z;
left_uv = gMyData.road_edge1_uv; right_uv = gMyData.road_edge2_uv;
% points
for i = 1:block.point_num/2
    j = 2*(i-1);
    block.point_array(j+1).position = [left(i,:),left_z(i)]';
    block.point_array(j+1).uv = left_uv(i,:)';
    block.point_array(j+2).position = [right(i,:),right_z(i)]';
    block.point_array(j+2).uv = right_uv(i,:)';    
end
% triangles
for i = 1:block.point_num/2-1
    j = 2*(i-1);
    block.tri_array(j+1).index = [j,j+1,j+2]';
    block.tri_array(j+2).index = [j+1,j+3,j+2]';
end
% end ref
block.endref=zeros(3,3);
deg = deg2rad(gMyData.direction);
[x,y] = pol2cart(deg,1);
forward = [x,y,0];
forward = forward/norm(forward);
up = [0,0,1];
block.endref(:,2) = forward';
block.endref(:,3) = up';
block.endref(:,1) = cross(forward,up)';

% end position
block.end_position = [mid(end,:),(left_z(end)+right_z(end))/2];

% -- generate texture coord
function generate_all_lines_uv()
global gMyData gHandles;
if get_triangle_num()==0
    return
end
gMyData.road_edge1_uv = generate_uv(gMyData.mid_line,gMyData.mid_line_z,0,gMyData.half_width*2);
gMyData.road_edge2_uv = generate_uv(gMyData.mid_line,gMyData.mid_line_z,1,gMyData.half_width*2);

function uvs = generate_uv(xys,zs,u,road_width)
xys(:,1) = xys(:,1) - xys(1,1);
xys(:,2) = xys(:,2) - xys(1,2);
zs = zs - zs(1);
for i=1:size(zs,1)
    lens(i) = norm([xys(i,:),zs(i)]);
end
len = size(lens,2);
uvs = zeros(len,2);
for i=1:len
    uvs(i,1) = u;
    v = lens(i)/road_width;
    uvs(i,2) = v;
end
    
% --- Outputs from this function are returned to the command line.
function varargout = road_editor_OutputFcn(hObject, eventdata, handles) 
global gMyData;
varargout{1} = handles.output;

% --- create edt_delta_angle
function edt_delta_angle_CreateFcn(hObject, eventdata, handles)
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end

% --- create edt_road_width
function edt_road_width_CreateFcn(hObject, eventdata, handles)
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end

% -- draw left,right,mid-line
function draw_all_lines()
global gMyData;
global X Y Z
contour(X,Y,Z);
'hi'
hold on;
left = gMyData.road_edge1; right = gMyData.road_edge2; mid = gMyData.mid_line;
plot(left(:,1),left(:,2),'bo');
plot(right(:,1),right(:,2),'b+');
plot(mid(:,1),mid(:,2),'r+');

% -- adjust axis limit according to road block
function update_axis_lim()
global gMyData;
global gHandles;
xs = [gMyData.road_edge1(end,1), gMyData.road_edge2(end,1), gMyData.min_x, gMyData.max_x];
ys = [gMyData.road_edge1(end,2), gMyData.road_edge2(end,2), gMyData.min_y, gMyData.max_y];
gMyData.min_x = min(xs);
gMyData.max_x = max(xs);
gMyData.min_y = min(ys);
gMyData.max_y = max(ys);
center.x = (gMyData.min_x+gMyData.max_x)/2;
center.y = (gMyData.min_y+gMyData.max_y)/2;
radius = max([gMyData.max_x-gMyData.min_x,gMyData.max_y-gMyData.min_y])/2;
h_axis = gHandles.road_show;
radius = radius * 1.2; % make axis range much large for better display
xlim = [center.x-radius,center.x+radius];
ylim = [center.y-radius,center.y+radius];
set(h_axis,'XLim',xlim,'YLim',ylim);

% -- clear make road data
function clear_all_lines_data()
global gMyData;
gMyData.road_edge1=[];
gMyData.road_edge2=[];
gMyData.road_edge1_z=[];
gMyData.road_edge2_z=[];
gMyData.mid_line=[];
gMyData.road_directions=[];
gMyData.select_h=[];
gMyData.select=[] ;

% -- initialize line
function init_lines()
global gMyData;
syn_edit_data();
clear_all_lines_data();
cla;
%initialize
gMyData.direction = 90;
gMyData.road_edge1(1,1)=-gMyData.half_width;
gMyData.road_edge1(1,2)=0;
gMyData.road_edge2(1,1)=gMyData.half_width;
gMyData.road_edge2(1,2)=0;
gMyData.mid_line(1,1) =0;
gMyData.mid_line(1,2)=0;
gMyData.road_directions(1) = gMyData.direction;
gMyData.min_x = -gMyData.half_width;
gMyData.max_x = gMyData.half_width;
gMyData.min_y = 0;
gMyData.max_y = 0;

% -- synchronize the controller's data
function syn_edit_data()
global gMyData;
global gHandles;
gMyData.delta_angle = str2num(get(gHandles.edt_delta_angle,'String'));
gMyData.half_width  = str2num(get(gHandles.edt_road_width,'String'))/2;
gMyData.delta_distance  = str2num(get(gHandles.edt_delta_dist,'String'));
gMyData.turn_move = str2num(get(gHandles.edt_turn_move,'String'));

% --- Executes on button press in btn_select.
function btn_select_Callback(hObject, eventdata, handles)
global gMyData gHandles;
if gMyData.is_drawing
    return
end
if ~isfield(gMyData,'road_edge1') 
    return
end
if length(gMyData.road_edge1)<2
    return
end
gMyData.is_selecting = ~gMyData.is_selecting;
if gMyData.is_selecting
    set(hObject,'String','Cancel');
else
    set(hObject,'String','Select');
end

% -- apply z-value
function btn_apply_z_Callback(hObject, eventdata, handles)
global gMyData gHandles;
if ~gMyData.is_selecting
    return
end
pt1_z = str2num(get(gHandles.pt1_z,'String'));
pt2_z = str2num(get(gHandles.pt2_z,'String'));
pt3_z = str2num(get(gHandles.pt3_z,'String'));
all_z = str2num(get(gHandles.all_sel_z,'String'));
if size(gMyData.select,2)==1
    set_z_value(1,1,pt1_z);
    set_z_value(1,2,pt2_z);
    set_z_value(1,3,pt3_z);
    draw_triangles();
    draw_select();
else
    for i = 1:size(gMyData.select,2)
        set_z_value(i,1,all_z);
        set_z_value(i,2,all_z);
        set_z_value(i,3,all_z);
    end
    draw_triangles();
    draw_all_select();
end

function set_z_value(sel_i,pt_number,z)
global gMyData;
indice = gMyData.select(sel_i).pt_index(pt_number,:);
range = gMyData.select(sel_i).pt_range(pt_number,:);
if indice(1)==1
    for i = range(1):range(2)
        gMyData.road_edge1_z(i) = z;
    end
else
    for i = range(1):range(2)
        gMyData.road_edge2_z(i) = z;
    end
end


% --- Executes on button press in btn_make_road.
function btn_make_road_Callback(hObject, eventdata, handles)
global gMyData gHandles;
gMyData.is_drawing = ~gMyData.is_drawing;
if gMyData.is_drawing
    set(gHandles.btn_make_road,'String','end to make road');
    init_lines();
    gMyData.is_selecting=false;
    update_road_show();
    set(gHandles.btn_select,'String','Select');
    init_select_uicontrol();
else
    % automatically connect first and last
    gMyData.road_edge1 = [gMyData.road_edge1;gMyData.road_edge1(1,:)];
    gMyData.road_edge2 = [gMyData.road_edge2;gMyData.road_edge2(1,:)];
    gMyData.mid_line = [gMyData.mid_line;gMyData.mid_line(1,:)];
    
    len = size(gMyData.road_edge1);
    len = len(1);
    % calculate the z value here, by Xian's method.
    global Z;
    for i=1:len
        x = gMyData.road_edge1(i,1);
        y = gMyData.road_edge1(i,2);
        gMyData.road_edge1_z(i,1) = Z( mod(round(x+150),300)+1, mod(round(y+150),300)+1);
        x = gMyData.road_edge2(i,1);
        y = gMyData.road_edge2(i,2);
        gMyData.road_edge2_z(i,1) = Z( mod(round(x+150),300)+1, mod(round(y+150),300)+1);
    end
    set(gHandles.btn_make_road,'String','start to make road');
    draw_triangles();
end

% --- draw road with triangles
function draw_triangles()
global gMyData;
left = gMyData.road_edge1;
left_z = gMyData.road_edge1_z;
right = gMyData.road_edge2;
right_z = gMyData.road_edge2_z;
cla;
len = size(left);
len = len(1);
% fill with color
hold on;
l = [left,left_z]; 
r = [right,right_z];
draw_roadblock(l,r,false);
colorbar;
hold off;

% --- Executes on key press over figure1 with no controls selected.
function MyKeyPressFcn(hObject,event)
global gMyData;
if ~gMyData.is_drawing
    return
end
syn_edit_data();
switch event.Key
    case 'uparrow'
        go_ahead();
    case 'leftarrow'
        turn_left();
    case 'rightarrow'
        turn_right();
    case 'downarrow'
        backspace();
end

% -- update the figure when drawing the track
function update_road_show()
cla;
update_axis_lim();
draw_all_lines();

% --- keypress callbacks
function go_ahead()
global gMyData;
deg = deg2rad(gMyData.direction);
[x,y] = pol2cart(deg,1);
vec = [x,y] * gMyData.delta_distance;
gMyData.road_edge1 = [gMyData.road_edge1;gMyData.road_edge1(end,:)+vec];
gMyData.road_edge2 = [gMyData.road_edge2;gMyData.road_edge2(end,:)+vec];
gMyData.mid_line   = [gMyData.mid_line  ;gMyData.mid_line(end,:)+vec];
gMyData.road_directions = [gMyData.road_directions; gMyData.direction];
update_road_show();

function is_warn = warning_turnmove()
global gMyData;
if gMyData.turn_move==0
    msgbox('turn move cannot be zero');
    is_warn = true;
    return
end
is_warn = false;

function turn_left()
if warning_turnmove()
    return
end
global gMyData;
gMyData.direction = gMyData.direction + gMyData.delta_angle;
deg = deg2rad(gMyData.direction);
[x,y] = pol2cart(deg-pi/2,1);
vec = [x,y]*gMyData.half_width;
[x,y] = pol2cart(deg,1);
mov = [x,y] * gMyData.turn_move;
pt = gMyData.road_edge1(end,:);
gMyData.road_edge1 = [gMyData.road_edge1;pt+mov];
gMyData.road_edge2 = [gMyData.road_edge2;pt+vec*2+mov];
gMyData.mid_line   = [gMyData.mid_line  ;pt+vec+mov];
gMyData.road_directions = [gMyData.road_directions; gMyData.direction];
update_road_show();

function turn_right()
if warning_turnmove()
    return
end
global gMyData;
gMyData.direction = gMyData.direction - gMyData.delta_angle;
deg = deg2rad(gMyData.direction);
[x,y] = pol2cart(deg+pi/2,1);
vec = [x,y]*gMyData.half_width;
[x,y] = pol2cart(deg,1);
mov = [x,y] * gMyData.turn_move;
pt = gMyData.road_edge2(end,:);
gMyData.road_edge1 = [gMyData.road_edge1;pt+vec*2+mov];
gMyData.road_edge2 = [gMyData.road_edge2;pt+mov];
gMyData.mid_line   = [gMyData.mid_line  ;pt+vec+mov];
gMyData.road_directions = [gMyData.road_directions; gMyData.direction];
update_road_show();

function backspace()
global gMyData
gMyData.road_edge1 = gMyData.road_edge1(1:end-1,:);
gMyData.road_edge2 = gMyData.road_edge2(1:end-1,:);
gMyData.mid_line   = gMyData.mid_line(1:end-1,:);
gMyData.road_directions = gMyData.road_directions(1:end-1,:);
gMyData.direction = gMyData.road_directions(end);
xs = [gMyData.road_edge1(:,1),gMyData.road_edge2(:,1)];
gMyData.min_x = min(xs);
gMyData.max_x = max(xs);
ys = [gMyData.road_edge1(:,2),gMyData.road_edge2(:,2)];
gMyData.min_y = min(ys);
gMyData.max_y = max(ys);
update_road_show();

% --- Executes during object creation, after setting all properties.
function btn_make_road_CreateFcn(hObject, eventdata, handles)
set(hObject,'KeyPressFcn',@MyKeyPressFcn);


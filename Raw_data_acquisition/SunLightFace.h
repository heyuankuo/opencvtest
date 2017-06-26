#pragma once
#include <windows.h>

//virtual BOOL InitInstance(); //DLL附加到进程时运行,初始化全局对象
//	virtual int ExitInstance();  //DLL释放资源

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//下面是人脸识别SDK的正式定义部分
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _USRDLL
       #define ZW_DEF  extern "C"  __declspec(dllexport)
#else
       #define ZW_DEF  extern "C"  __declspec(dllimport)
#endif

//人脸检测的输出结果结构
struct DLL_OUT_FACE_STRUCT
{
   BYTE address;     //无意义，传址的
   LONG eye1_x;
   LONG eye1_y;

   LONG eye2_x;
   LONG eye2_y;       //两眼坐标,要进行了人眼定位才有效

   LONG left;
   LONG top;
   LONG right;
   LONG bottom;       //人脸矩形

   LONG angle;        //人脸平面角度 (正面垂直时为90度)
   float tally;       //得分 ,100分制,即人脸的置信度。

   LONG is_small_face;   //是小脸还是大脸，1为小脸，是启用了小脸检测的结果，但小脸不能进行后续的人脸识别
                         //当是小脸时，只输出人脸矩形，即只有人脸矩形有效,其它值无效

   //下面的值要进行了人眼定位与人脸检测的后期处理才有效
   LONG skin_color_R;      //采样肤色COLORREF。(RGB)
   LONG skin_color_G;      //采样肤色COLORREF。(RGB)
   LONG skin_color_B;      //采样肤色COLORREF。(RGB)
   LONG skin_hd_bright;    //采样肤色的灰度亮度。

	LONG left_face_len;       //从左眼开始计算的左脸估计长度。
	LONG right_face_len;      //从右眼开始计算的右脸估计长度。
	float face_width_rely;     //脸宽的信任度[0，1）。

	LONG nose_x;                  //在原图像中的鼻尖位置。
	LONG nose_y;                  //在原图像中的鼻尖位置。
	float nose_rely;              //鼻尖位置:可信任度[0，1）。

	LONG month_x;                 //在原图像中的嘴中心位置。
	LONG month_y;                 //在原图像中的嘴中心位置。
	float month_rely;             //嘴心位置:可信任度[0，1）。

	float glass_rely;             //可能眼镜的置信度[0,1)
  
	
	LONG eye1_w;  //人眼的两个眼球的宽与高,人眼的两个眼球的宽与高,但要求姿态端正，眼距100以上，无反光的情况下才正确。
	LONG eye1_h;
	LONG eye2_w;
	LONG eye2_h;

	LONG CloseEyeBelievable;           //闭眼的可能性系数输出,值域[0,1000]

};//END STRUCT DEF


//1CN的识别输出结构
struct DLL_1CN_RECOG_OUT_STRUCT
{
	 BYTE address;     //无意义，传址的
     float value;
	 char Template_ID[33];
	 char TemplateFileName[256];
};

//一般地，SDK提供的所有函数的返回值为0，表示失败或消极结果，小于0则表示传入参数发生错误，1和1以上表示函数成功。
ZW_DEF LONG __stdcall FaceCalcData();  //假打
ZW_DEF LONG __stdcall zInitialize(CHAR *username);                //装入SDK时最先执行且只执行一次的。
ZW_DEF LONG __stdcall zUnInitialize();                         //卸掉SDK进要执行一次。
ZW_DEF LONG __stdcall zGetLastError(LONG OID,CHAR *msg);    //取得最后一次失败的原因信息。
ZW_DEF LONG __stdcall zCreateOneThreadObject(LONG IS_LOAD_TZLIB=1,CHAR *DataFilePath=NULL);     
//参数1表示是否载入特征库到内存，只做慢速的1C1(或只做人脸检测)就不用LOAD.
//参数2表示指明数据文件的路径，数据与程序可以不在同一目录中，如果在同一目录，就默认为空。
//返回值，LONG 线程对象的ID，线程ID值域[1,64],共可建64个线程,0表示失败
//如果是多线程，则此函数必要在线程上运行，即，一个对象实例，从头到尾都只能在一个线程上建立，运行，销毁。

ZW_DEF LONG __stdcall zDeleteOneThreadObject(LONG OID);   //返回值0，失败，1，成功

////////////////////////
//上面是初始化部分
//下面是人脸识别主体部分
////////////////////////

//人脸定位的输出数组已按从高到低的得分排了序。
ZW_DEF LONG __stdcall zFaceLocate(  LONG OID,                     //第一个函数返回的人脸识别实例对象ID
		                            CHAR *FileName,               //图像文件名，JPG,BMP
									LONG max_out_nums,            //用户要求的最大人脸输出数
									FLOAT Threshold,              //人脸置信度阀值，高于这个阀值才会被输出
                                    DLL_OUT_FACE_STRUCT *dofs);   //人脸输出结构数组
                                                                  //返回值，实际输出的人脸数量
   
ZW_DEF LONG __stdcall zFaceLocate_BmpData(  LONG OID,                     //第一个函数返回的人脸识别实例对象ID
		                                   BYTE *BmpData,INT width,INT height,INT bitcount,     //标准Windows位图数据(lpData,320,240,24)
									       LONG max_out_nums,            //用户要求的最大人脸输出数
									       FLOAT Threshold,              //人脸置信度阀值，高于这个阀值才会被输出
                                           DLL_OUT_FACE_STRUCT *dofs);    //人脸输出结构数组
                                                                         //返回值，实际输出的人脸数量,如果一张照片上有8张人脸，则返回8.


ZW_DEF LONG __stdcall zFlagFace( LONG OID,LONG draw_window_hwnd,LONG order,LONG offset_x=0,LONG offset_y=0);//在目标窗口上标志人脸
ZW_DEF LONG __stdcall zFaceLocate_FreeMemory( LONG OID);//人脸检测完成后，可用作人脸识别，或画标志，最后在清除人脸检测暂用的内存。


//注意：识别一般可持续进行3次，以提高识别的稳定性。
ZW_DEF LONG __stdcall zRecog1C1( LONG OID,CHAR *VID,LONG order,FLOAT *value,CHAR *TemplateFileName=NULL);
ZW_DEF LONG __stdcall zRecog1C1_Fast( LONG OID,CHAR *VID,LONG order,FLOAT *value,CHAR *TemplateFileName=NULL);
//VID是指当前检测到的人脸和模板库中的哪一个ID的脸做验证。
//order是人脸检测中输出的编号，order>=0 order<人脸检测输出人脸数.
//比如说要对一张照片上检测出的8张人脸全都做识别，order从0到7，调用8次人脸验证或识别函数即可。

//value是本函数输出的相似度，值域[0，1）
//TemplateFileName请先预置256BYTE的空间，由于一个ID有多个模板，这里是输出与当前脸最像的模板的源图文件名称。
//FAST_1C1在速度上比1C1快并不是绝对的，当库中模板数越多时FAST_1C1会渐渐变慢，比如万人，或十万人，或更多，有可能FAST_1C1反而比1C1慢。
//因为1C1用的是标准SQL数据库查找模式（并比对TID对应的所有模板），而FAST_1C1用的是内存中逐一检索模式（只比对TID中最相似的两三个模板）。

ZW_DEF LONG __stdcall zRecog1CN( LONG OID,LONG order,LONG max_out_num,DLL_1CN_RECOG_OUT_STRUCT *dros);
//order是人脸检测中输出的编号，order>=0 order<输出人脸数.
//max_out_num是指输出多少个与ORDER相似的人脸，返回值就是实际输出数
//dros是输出结构

//////////////////////
//下面是赠送的视频函数
//////////////////////
ZW_DEF LONG __stdcall UsbVideo_Init(LONG DEVICE_ID,LONG play_window_hwnd);
ZW_DEF LONG __stdcall UsbVideo_CapOneBmp(CHAR *BmpFileName);
ZW_DEF LONG __stdcall UsbVideo_EndAll();

/////////////////////
//下面是模板管理函数
/////////////////////

ZW_DEF LONG __stdcall zAddFaceTemplate(LONG OID,CHAR *TID,LONG order);
//TID是指要加入的模板ID,一个模板ID可以加入多个模板。
//order是人脸检测结果中的脸序号

//以下所有删除函数的返回值是删除的记录个数
ZW_DEF LONG __stdcall zDelTemplateA(LONG OID,CHAR *TID,CHAR *template_filename=NULL);	
//如果没有提供模板文件名，就删除这个模板ID对应的所有模板，否则，只删除那一张。
ZW_DEF LONG __stdcall zDelTemplateB(LONG OID,CHAR *TID,LONG BH); //BH是从0开始的,不从1开始
//删除这个模板ID对应的时序中的第BH张模板
ZW_DEF LONG __stdcall zDelAllTemplate(LONG OID);
//删除模板库的所有模板


ZW_DEF LONG __stdcall zUpdateMemory(LONG OID);
//因为前面的操作都是对数据库的，所以要让变更立即有效，就要读库到内存，否则，变更只有下次重启软件后才有效。
//返回值是当前内存中的模板总数
//如果只做慢速的1C1,（或只做人脸检测），则 zCreateOneThreadObject(LONG IS_LOAD_TZLIB=0,...)，同时，不必以后使用任何的zUpdateMemory,因为慢速1C1不使用载入内存的模板特征库。
//如果只新增了一个模板，要让它立生效（对1C1_FAST和1CN），则调用本函数。但如果是批量增加模板，则在大量增加之后，调用一次本函数即可，否则会大大浪费时间。

ZW_DEF LONG __stdcall zCountMemoryTidTotaleNums(LONG OID); 
//统计内存中模板ID总数，因为一个TID（模板ID）可能有多个模板

ZW_DEF LONG __stdcall zCountTemplateTotaleNums(LONG OID,CHAR *TID=NULL); 
//统计数据库中所有模板总数，如果TID不为空，则是指这个TID的模板总数

ZW_DEF LONG __stdcall zDrawOneTemplatePhoto(LONG OID,CHAR *TID,LONG BH,LONG object_window_hwnd,LONG start_x=0,LONG start_y=0,LONG IS_FLAG_EYE=1);//此函数要与上一函数结合使用
//TID是模板ID
//BH是指画这个模板ID的第几张照片
//object_window_hwnd是要画的目标窗口
//start_x,start_y是照片的左上角坐标
//IS_FALG_EYE是指是否标出眼球,如果一个照片上的眼球标错，大可以删除这张照片对应的模板，因为可能导致识别出错!


//////////////////////////////////
//下面是输出调试变量
//////////////////////////////////
ZW_DEF LONG __stdcall zGetA(LONG OID,LONG PARA_NAME_ORDER);
//下面将返回最近一次的功能函数执行所用的时间：单位ms
#define de_out_recog_1cn_use_time 1  //zGetA(OID,de_out_recog_1cn_use_time)返回识别函数所用的毫秒数(不包含人脸检测的)
#define de_out_recog_1c1_use_time  2   //1C1
#define de_out_recog_fast_1c1_use_time 3  //快速1C1
#define de_out_find_face_use_time 4     //人脸检测
#define de_out_add_template_use_time 5  //添加模板
#define de_out_del_template_use_time 6  //返回任何一种删除模板 函数所用的时间
#define de_out_update_memory_use_time 7  //返回更新内存模板特征 函数所用的时间,注意，模板越多，速度越慢

#define de_out_is_template_update_happen_in_recog 8//最近的一次人脸识别或认证有无发生《动态模板更新》,如果发生:
//如果后续进行的是1CN或FAST_1C1,建议可以进行一次UpdateMemory(),使当前更新立即生效。但也要注意UpdateMemory()在模板上千后会很慢。


//////////////////////////////////
//下面是参数设置部分(一般不必设置)
//////////////////////////////////
ZW_DEF LONG __stdcall zSetA(LONG OID,LONG PARA_NAME_ORDER,LONG VALUE);  //BOOL型及整形参数设置
ZW_DEF LONG __stdcall zSetB(LONG OID,LONG PARA_NAME_ORDER,FLOAT VALUE); //小数型的参数设置
//PARA_NAME_ORDER 是参数的宏名称（数字编号）
//下面就是具体的定义：

//初始值：是指建立人脸识别对象时就默认已有的设置值，用户可根据实际情况作一些修改。
//LONG型就调用函数A,FLOAT型就调用函数B

#define de_is_auto_backlighting_repair 1  //LONG型,初始值1
//是否开启自动逆光补偿（又名背光补偿）功能。
//对于门禁，人脸电脑密码等，可能让摄像头背光（逆光）的地方，即人脸暗，但光线在背后，且很亮，采用。
//注意：在背景暗，人脸亮的地方采用，会起很大的负面效果。即正常光线情况下建议不采用。
//注意：背光补充的有效区在画面的中下方,就是除掉画面的上1/3，左1/3，右1/3后的余下部分。所认为的光源是在画面的上1/3处。
#define de_ZW_HALF_UP_LIGHT_LD_MIN_VALUE 1001 //LONG,初始值180,当开启了背光补偿功能功能后，图像上方1/3最小亮度要大于这个值才能做背光补偿
#define de_ZW_HALF_UP_LIGHT_LD_MIN_BL 1002    //FLOAT,初始值1.8f,当开启了背光补偿功能功能后，上1/3的亮度比中1/3的亮度高多少倍才能做背光补偿
#define de_backlighting_repair_base_value 2   //LONG,初始值33，最大值100
//背光补偿:补多少?



#define de_is_green_eye_ball_optimize 3 //是否绿眼球化化,LONG型,初始值1
#define de_is_blue_eye_ball_optimize  4  //是否蓝眼球优化,LONG型,初始值1
//优点是利于这两种色的眼球的定位(有利于人眼定位的精确性，尤其是有非黑眼球人的情况下)
//注意：在绿光，蓝光特重的灯光下，人脸呈现绿色或蓝色时，易出现人眼定位错，从而使人脸识别错。（不影响人脸检测）
//初值为1，主要是用于有外国人，但没有绿蓝光照的场合


#define de_is_second_locate_eye 5           //进行人眼定位,LONG型,初始值1
#define de_is_second_locate_eye_infection 6 //进行人眼定位后的后期计算（如是否戴眼镜的判断）,只有上一个开关打开的情况下，本开关才有效。LONG型,初始值1
//如果要进行人脸识别与验证，这两个开关都必要打开。
//如果只是尽可能多地捕捉人脸，这两个开关最好关掉，以节省时间。

//下面的六项开关，依赖于上一个开关的打开才有效
#define de_is_second_locate_eye_infection_GETFACE  6001  //LONG型,初始值0,面膜提取,只有上一个开关打开的情况下，本开关才有效。
#define de_is_second_locate_eye_infection_GETFACE_Threshold  6002  //LONG型,初始值64,但依赖于上一个开关的开启才有效
#define de_is_second_locate_eye_infection_CALC_EYEWHITE  6003 ////LONG型,初始值0 是否启动眼白运算（若要进行活体验证或打磕睡检测）

#define de_is_second_locate_eye_infection_FOR_RECOG_OR_GLASS  6004 //LONG型,初始值1 是否为的是：人脸识别或眼镜判定,如果下一步要采模板或识别，此项必要。
#define de_is_second_locate_eye_infection_FACE_BORDER  6005        //LONG型,初始值1 是否启动人脸边界查找。
#define de_is_second_locate_eye_infection_FACE_NOSE_MONTH  6006    //LONG型,初始值1 是否启动鼻嘴查找。


#define de_vertical_angle_limit 7 //LONG型,初始值60
//人脸检测中平面旋转的角度范围
//一般地，监控可为45度，识别30度(可以省时，可以降低错捉率，机会多也就意味着错误也多,很多歪脸都是错捉的)。
//注：最大可设置到80度，但人脸检测的速度会相对变慢

#define de_automode 8  //LONG型,初始值1
//人脸检测自动模式,自动模式下用的时间可以少些，但在多张脸同时存在时，有较小可能漏捉人脸。

#define de_is_run_smalle_face 9  //LONG型,初始值0
//极小脸是否启动的"全局总开关",		
//当最终用户的要求及格分在80分以上时，建议关小脸，小脸检测无意义，白浪费大量时间，因为其分率只在50-80分段，上70分的都只是极少数。
//所以此时小脸检测自行关闭，以减少时间占用。
//当用于人脸识别时，建议关小脸，小脸检测只在进行高灵敏度的只捉人脸的监控时启用。

#define de_is_color_filter 10             //LONG型,初始值1,肤色过滤 默认是打开的(为提速)，只在酒吧或舞厅关掉，这时肤色是乱的。
#define de_color_filter_min_bmp_width 11  //LONG型,初始值300,作肤色过滤的最小位图宽，因为对于一些小的位图，没有必要做肤色过滤。


#define de_face_locate_zdz  12   //FLOAT型,初值 0.0783f  （第一次查找人脸时的）
#define de_face_locate_bank_zdz 13//FLOAT型,初值 0.0525f  （第二次查找人脸时的，在自动人脸检测模式下，第一次未找到及格人脸，会发动第二次）
#define de_face_is_use_bank 131   //LONG,1,是否启动第二次查找，用de_face_locate_bank_zdz这个值，对自动模式和非自动模式都有效。
//在人脸检测中，查找人脸内部轮廓的边缘检测阀值，有时一张图有人脸却找不到，略改一点儿就找到了，这两个预设值代表大多数测试的经验最优值。


//下面主要针对导出的zFlagFace函数而言
//目前，下面这些定位的精度是有条件的，只有在人脸正，光线正的条件下才能保证准确度，否则只能是个大约的范围估计。
#define de_flagface_draw_eye 14      //LONG型,初始值1,是否画眼
#define de_is_draw_face_border 15    //LONG型,初始值1,是否画脸边界
#define de_is_draw_nose_month 16     //LONG型,初始值1,是否标志鼻嘴
#define de_draw_nose_month_rely 17   //FLOAT,初始值0.3f,画出鼻嘴的最低置信度     
#define de_draw_face_width_rely 18   //FLOAT ,初始值0.4f,画出脸宽(脸边界)的最低置信度
#define de_is_have_glass_threshold 19//FLOAT ,初始值0.65f; 画出眼镜标记的最低置信度,无开关，但高于1.0f就不画了


//下面是关于模板管理的
//关于模板更新
		#define de_is_adding_template_scroll  20//LONG型,初始值1
		//在添加模板时，人脸模板是否超过限制量就滚动。如果未启动滚动，过量就无法再加入。	
				
		//如果启动了滚动更新，当到达最大数量时，会自动删除掉最早的模板，以维持模板总量不变。
		#define de_template_limite_num  21//LONG型,初始值10,即每个人最多10个模板
		#define de_template_scroll_reserve_num   22//LONG型,初始值5     
        //保持多少个最早的模板，0，不保持，3，则表示最早的3个始终不被滚动更新掉。

		#define de_is_template_recog_adding_update  23//LONG型,初始值0,
        //识别发生后,相似度过指定门值,是否将当前人脸加为模板。
		#define de_is_template_recog_hit_update   24 //LONG型,初始值0,
        //识别发生后，相似度过指定门值,是否用当前人脸为模板替换掉与当前人脸最像的那个模板。
		//注意：这两个BOOL是由开发人员保持其设置为互斥的。

		#define de_template_scroll_threshold   25 //FLOAT,初值0.85f;即人脸识别相似度大于了85%,就自动更新模板
        //在用户添加模板时，这个值无意义。这个值是出厂默认值，但最终应由用户根据实际情况而定。		
        #define de_template_hit_threshold    26  //FLOAT,初值0.85f;即人脸识别相似度大于了85%,就自动更新模板		
		//在识别时，相似度大于了更新门值，才可以更新模板
		//END 关于模板更新
 

//下面是其它的:

	#define de_1CN_first_filter_reserve_bl  27 //FLOAT,初值0.02f
    //1CN在内存特征中进行过滤后的保留比  //(2%)
	//在内存人脸特征的过滤中,只保留2%的模板来做一一详细比对，这个值改小，会提速，尤其是在千人库，万人库中，但漏识率增加。
   

    //活体识别时人脸不能太小，眼距应大于40pixel, 闭眼瞬间光线不能闪烁
	#define de_check_eye_hd_change_bl  28 //FLOAT,0.45f; //眼球的“变化率”必要大于这个值，才认为眨了眼。
	//实测当一张照片不动时，变化度冲到最高0.23f.
	//眼球的“亮度变化率”必要大于这个值，才认为眨了眼。
	//当影像的对比度，清晰度越高时，为加大活体识别的可靠性，建议调高此值。比如取0.55，反之，取0.35
	//如果明明闭了眼，却没有活动提示，最终超时（活体识别一直返回0），说明这个值应调小，增大灵敏度
	//反之，如果值太小，没动眼也会误认为动了眼（别人的照片抖一下就能过）（活体识别返回1）。
	//原理是：因眼皮比眼球更亮
    #define	de_check_eye_face_change_Threshold 29 //FLOAT,0.33f;//人脸的二值稳定度要大于这个值才行
    //人脸稳定度，即只准眼变，其它地方不许可变，目的在于防止照片平移，如果人脸或照片不闭眼，但平移一下可过，加大这个值。
    //但人脸明明未平移却返回-1，则要调低这个值。



//下为活体识别的两条函数
ZW_DEF LONG __stdcall zInitCheck(LONG OID,LONG order);
//初始化活体识别现场函数，order是人脸检测结果序,一般取第一个：0


ZW_DEF LONG __stdcall zFrameCheck(LONG OID,CHAR *bmpFileName);//注意，这里传入的一定要是BMP格式的文件，不能用JPG
//帧检测函数，返回0时，请再执行，表示还未能判断出来，返回小于0，表示活体识别不能通过，是作弊。返回1，表示用户是活体。有闭一下眼。



#pragma once
#include <windows.h>

//virtual BOOL InitInstance(); //DLL���ӵ�����ʱ����,��ʼ��ȫ�ֶ���
//	virtual int ExitInstance();  //DLL�ͷ���Դ

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//����������ʶ��SDK����ʽ���岿��
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef _USRDLL
       #define ZW_DEF  extern "C"  __declspec(dllexport)
#else
       #define ZW_DEF  extern "C"  __declspec(dllimport)
#endif

//���������������ṹ
struct DLL_OUT_FACE_STRUCT
{
   BYTE address;     //�����壬��ַ��
   LONG eye1_x;
   LONG eye1_y;

   LONG eye2_x;
   LONG eye2_y;       //��������,Ҫ���������۶�λ����Ч

   LONG left;
   LONG top;
   LONG right;
   LONG bottom;       //��������

   LONG angle;        //����ƽ��Ƕ� (���洹ֱʱΪ90��)
   float tally;       //�÷� ,100����,�����������Ŷȡ�

   LONG is_small_face;   //��С�����Ǵ�����1ΪС������������С�����Ľ������С�����ܽ��к���������ʶ��
                         //����С��ʱ��ֻ����������Σ���ֻ������������Ч,����ֵ��Ч

   //�����ֵҪ���������۶�λ���������ĺ��ڴ������Ч
   LONG skin_color_R;      //������ɫCOLORREF��(RGB)
   LONG skin_color_G;      //������ɫCOLORREF��(RGB)
   LONG skin_color_B;      //������ɫCOLORREF��(RGB)
   LONG skin_hd_bright;    //������ɫ�ĻҶ����ȡ�

	LONG left_face_len;       //�����ۿ�ʼ������������Ƴ��ȡ�
	LONG right_face_len;      //�����ۿ�ʼ������������Ƴ��ȡ�
	float face_width_rely;     //��������ζ�[0��1����

	LONG nose_x;                  //��ԭͼ���еıǼ�λ�á�
	LONG nose_y;                  //��ԭͼ���еıǼ�λ�á�
	float nose_rely;              //�Ǽ�λ��:�����ζ�[0��1����

	LONG month_x;                 //��ԭͼ���е�������λ�á�
	LONG month_y;                 //��ԭͼ���е�������λ�á�
	float month_rely;             //����λ��:�����ζ�[0��1����

	float glass_rely;             //�����۾������Ŷ�[0,1)
  
	
	LONG eye1_w;  //���۵���������Ŀ����,���۵���������Ŀ����,��Ҫ����̬�������۾�100���ϣ��޷��������²���ȷ��
	LONG eye1_h;
	LONG eye2_w;
	LONG eye2_h;

	LONG CloseEyeBelievable;           //���۵Ŀ�����ϵ�����,ֵ��[0,1000]

};//END STRUCT DEF


//1CN��ʶ������ṹ
struct DLL_1CN_RECOG_OUT_STRUCT
{
	 BYTE address;     //�����壬��ַ��
     float value;
	 char Template_ID[33];
	 char TemplateFileName[256];
};

//һ��أ�SDK�ṩ�����к����ķ���ֵΪ0����ʾʧ�ܻ����������С��0���ʾ���������������1��1���ϱ�ʾ�����ɹ���
ZW_DEF LONG __stdcall FaceCalcData();  //�ٴ�
ZW_DEF LONG __stdcall zInitialize(CHAR *username);                //װ��SDKʱ����ִ����ִֻ��һ�εġ�
ZW_DEF LONG __stdcall zUnInitialize();                         //ж��SDK��Ҫִ��һ�Ρ�
ZW_DEF LONG __stdcall zGetLastError(LONG OID,CHAR *msg);    //ȡ�����һ��ʧ�ܵ�ԭ����Ϣ��
ZW_DEF LONG __stdcall zCreateOneThreadObject(LONG IS_LOAD_TZLIB=1,CHAR *DataFilePath=NULL);     
//����1��ʾ�Ƿ����������⵽�ڴ棬ֻ�����ٵ�1C1(��ֻ���������)�Ͳ���LOAD.
//����2��ʾָ�������ļ���·���������������Բ���ͬһĿ¼�У������ͬһĿ¼����Ĭ��Ϊ�ա�
//����ֵ��LONG �̶߳����ID���߳�IDֵ��[1,64],���ɽ�64���߳�,0��ʾʧ��
//����Ƕ��̣߳���˺�����Ҫ���߳������У�����һ������ʵ������ͷ��β��ֻ����һ���߳��Ͻ��������У����١�

ZW_DEF LONG __stdcall zDeleteOneThreadObject(LONG OID);   //����ֵ0��ʧ�ܣ�1���ɹ�

////////////////////////
//�����ǳ�ʼ������
//����������ʶ�����岿��
////////////////////////

//������λ����������Ѱ��Ӹߵ��͵ĵ÷�������
ZW_DEF LONG __stdcall zFaceLocate(  LONG OID,                     //��һ���������ص�����ʶ��ʵ������ID
		                            CHAR *FileName,               //ͼ���ļ�����JPG,BMP
									LONG max_out_nums,            //�û�Ҫ���������������
									FLOAT Threshold,              //�������Ŷȷ�ֵ�����������ֵ�Żᱻ���
                                    DLL_OUT_FACE_STRUCT *dofs);   //��������ṹ����
                                                                  //����ֵ��ʵ���������������
   
ZW_DEF LONG __stdcall zFaceLocate_BmpData(  LONG OID,                     //��һ���������ص�����ʶ��ʵ������ID
		                                   BYTE *BmpData,INT width,INT height,INT bitcount,     //��׼Windowsλͼ����(lpData,320,240,24)
									       LONG max_out_nums,            //�û�Ҫ���������������
									       FLOAT Threshold,              //�������Ŷȷ�ֵ�����������ֵ�Żᱻ���
                                           DLL_OUT_FACE_STRUCT *dofs);    //��������ṹ����
                                                                         //����ֵ��ʵ���������������,���һ����Ƭ����8���������򷵻�8.


ZW_DEF LONG __stdcall zFlagFace( LONG OID,LONG draw_window_hwnd,LONG order,LONG offset_x=0,LONG offset_y=0);//��Ŀ�괰���ϱ�־����
ZW_DEF LONG __stdcall zFaceLocate_FreeMemory( LONG OID);//���������ɺ󣬿���������ʶ�𣬻򻭱�־��������������������õ��ڴ档


//ע�⣺ʶ��һ��ɳ�������3�Σ������ʶ����ȶ��ԡ�
ZW_DEF LONG __stdcall zRecog1C1( LONG OID,CHAR *VID,LONG order,FLOAT *value,CHAR *TemplateFileName=NULL);
ZW_DEF LONG __stdcall zRecog1C1_Fast( LONG OID,CHAR *VID,LONG order,FLOAT *value,CHAR *TemplateFileName=NULL);
//VID��ָ��ǰ��⵽��������ģ����е���һ��ID��������֤��
//order���������������ı�ţ�order>=0 order<����������������.
//����˵Ҫ��һ����Ƭ�ϼ�����8������ȫ����ʶ��order��0��7������8��������֤��ʶ�������ɡ�

//value�Ǳ�������������ƶȣ�ֵ��[0��1��
//TemplateFileName����Ԥ��256BYTE�Ŀռ䣬����һ��ID�ж��ģ�壬����������뵱ǰ�������ģ���Դͼ�ļ����ơ�
//FAST_1C1���ٶ��ϱ�1C1�첢���Ǿ��Եģ�������ģ����Խ��ʱFAST_1C1�ὥ���������������ˣ���ʮ���ˣ�����࣬�п���FAST_1C1������1C1����
//��Ϊ1C1�õ��Ǳ�׼SQL���ݿ����ģʽ�����ȶ�TID��Ӧ������ģ�壩����FAST_1C1�õ����ڴ�����һ����ģʽ��ֻ�ȶ�TID�������Ƶ�������ģ�壩��

ZW_DEF LONG __stdcall zRecog1CN( LONG OID,LONG order,LONG max_out_num,DLL_1CN_RECOG_OUT_STRUCT *dros);
//order���������������ı�ţ�order>=0 order<���������.
//max_out_num��ָ������ٸ���ORDER���Ƶ�����������ֵ����ʵ�������
//dros������ṹ

//////////////////////
//���������͵���Ƶ����
//////////////////////
ZW_DEF LONG __stdcall UsbVideo_Init(LONG DEVICE_ID,LONG play_window_hwnd);
ZW_DEF LONG __stdcall UsbVideo_CapOneBmp(CHAR *BmpFileName);
ZW_DEF LONG __stdcall UsbVideo_EndAll();

/////////////////////
//������ģ�������
/////////////////////

ZW_DEF LONG __stdcall zAddFaceTemplate(LONG OID,CHAR *TID,LONG order);
//TID��ָҪ�����ģ��ID,һ��ģ��ID���Լ�����ģ�塣
//order������������е������

//��������ɾ�������ķ���ֵ��ɾ���ļ�¼����
ZW_DEF LONG __stdcall zDelTemplateA(LONG OID,CHAR *TID,CHAR *template_filename=NULL);	
//���û���ṩģ���ļ�������ɾ�����ģ��ID��Ӧ������ģ�壬����ֻɾ����һ�š�
ZW_DEF LONG __stdcall zDelTemplateB(LONG OID,CHAR *TID,LONG BH); //BH�Ǵ�0��ʼ��,����1��ʼ
//ɾ�����ģ��ID��Ӧ��ʱ���еĵ�BH��ģ��
ZW_DEF LONG __stdcall zDelAllTemplate(LONG OID);
//ɾ��ģ��������ģ��


ZW_DEF LONG __stdcall zUpdateMemory(LONG OID);
//��Ϊǰ��Ĳ������Ƕ����ݿ�ģ�����Ҫ�ñ��������Ч����Ҫ���⵽�ڴ棬���򣬱��ֻ���´�������������Ч��
//����ֵ�ǵ�ǰ�ڴ��е�ģ������
//���ֻ�����ٵ�1C1,����ֻ��������⣩���� zCreateOneThreadObject(LONG IS_LOAD_TZLIB=0,...)��ͬʱ�������Ժ�ʹ���κε�zUpdateMemory,��Ϊ����1C1��ʹ�������ڴ��ģ�������⡣
//���ֻ������һ��ģ�壬Ҫ��������Ч����1C1_FAST��1CN��������ñ����������������������ģ�壬���ڴ�������֮�󣬵���һ�α��������ɣ���������˷�ʱ�䡣

ZW_DEF LONG __stdcall zCountMemoryTidTotaleNums(LONG OID); 
//ͳ���ڴ���ģ��ID��������Ϊһ��TID��ģ��ID�������ж��ģ��

ZW_DEF LONG __stdcall zCountTemplateTotaleNums(LONG OID,CHAR *TID=NULL); 
//ͳ�����ݿ�������ģ�����������TID��Ϊ�գ�����ָ���TID��ģ������

ZW_DEF LONG __stdcall zDrawOneTemplatePhoto(LONG OID,CHAR *TID,LONG BH,LONG object_window_hwnd,LONG start_x=0,LONG start_y=0,LONG IS_FLAG_EYE=1);//�˺���Ҫ����һ�������ʹ��
//TID��ģ��ID
//BH��ָ�����ģ��ID�ĵڼ�����Ƭ
//object_window_hwnd��Ҫ����Ŀ�괰��
//start_x,start_y����Ƭ�����Ͻ�����
//IS_FALG_EYE��ָ�Ƿ�������,���һ����Ƭ�ϵ������������ɾ��������Ƭ��Ӧ��ģ�壬��Ϊ���ܵ���ʶ�����!


//////////////////////////////////
//������������Ա���
//////////////////////////////////
ZW_DEF LONG __stdcall zGetA(LONG OID,LONG PARA_NAME_ORDER);
//���潫�������һ�εĹ��ܺ���ִ�����õ�ʱ�䣺��λms
#define de_out_recog_1cn_use_time 1  //zGetA(OID,de_out_recog_1cn_use_time)����ʶ�������õĺ�����(��������������)
#define de_out_recog_1c1_use_time  2   //1C1
#define de_out_recog_fast_1c1_use_time 3  //����1C1
#define de_out_find_face_use_time 4     //�������
#define de_out_add_template_use_time 5  //���ģ��
#define de_out_del_template_use_time 6  //�����κ�һ��ɾ��ģ�� �������õ�ʱ��
#define de_out_update_memory_use_time 7  //���ظ����ڴ�ģ������ �������õ�ʱ��,ע�⣬ģ��Խ�࣬�ٶ�Խ��

#define de_out_is_template_update_happen_in_recog 8//�����һ������ʶ�����֤���޷�������̬ģ����¡�,�������:
//����������е���1CN��FAST_1C1,������Խ���һ��UpdateMemory(),ʹ��ǰ����������Ч����ҲҪע��UpdateMemory()��ģ����ǧ��������


//////////////////////////////////
//�����ǲ������ò���(һ�㲻������)
//////////////////////////////////
ZW_DEF LONG __stdcall zSetA(LONG OID,LONG PARA_NAME_ORDER,LONG VALUE);  //BOOL�ͼ����β�������
ZW_DEF LONG __stdcall zSetB(LONG OID,LONG PARA_NAME_ORDER,FLOAT VALUE); //С���͵Ĳ�������
//PARA_NAME_ORDER �ǲ����ĺ����ƣ����ֱ�ţ�
//������Ǿ���Ķ��壺

//��ʼֵ����ָ��������ʶ�����ʱ��Ĭ�����е�����ֵ���û��ɸ���ʵ�������һЩ�޸ġ�
//LONG�;͵��ú���A,FLOAT�;͵��ú���B

#define de_is_auto_backlighting_repair 1  //LONG��,��ʼֵ1
//�Ƿ����Զ���ⲹ�����������ⲹ�������ܡ�
//�����Ž���������������ȣ�����������ͷ���⣨��⣩�ĵط��������������������ڱ����Һ��������á�
//ע�⣺�ڱ��������������ĵط����ã�����ܴ�ĸ���Ч������������������½��鲻���á�
//ע�⣺���ⲹ�����Ч���ڻ�������·�,���ǳ����������1/3����1/3����1/3������²��֡�����Ϊ�Ĺ�Դ���ڻ������1/3����
#define de_ZW_HALF_UP_LIGHT_LD_MIN_VALUE 1001 //LONG,��ʼֵ180,�������˱��ⲹ�����ܹ��ܺ�ͼ���Ϸ�1/3��С����Ҫ�������ֵ���������ⲹ��
#define de_ZW_HALF_UP_LIGHT_LD_MIN_BL 1002    //FLOAT,��ʼֵ1.8f,�������˱��ⲹ�����ܹ��ܺ���1/3�����ȱ���1/3�����ȸ߶��ٱ����������ⲹ��
#define de_backlighting_repair_base_value 2   //LONG,��ʼֵ33�����ֵ100
//���ⲹ��:������?



#define de_is_green_eye_ball_optimize 3 //�Ƿ������򻯻�,LONG��,��ʼֵ1
#define de_is_blue_eye_ball_optimize  4  //�Ƿ��������Ż�,LONG��,��ʼֵ1
//�ŵ�������������ɫ������Ķ�λ(���������۶�λ�ľ�ȷ�ԣ��������зǺ������˵������)
//ע�⣺���̹⣬�������صĵƹ��£�����������ɫ����ɫʱ���׳������۶�λ���Ӷ�ʹ����ʶ�������Ӱ��������⣩
//��ֵΪ1����Ҫ������������ˣ���û���������յĳ���


#define de_is_second_locate_eye 5           //�������۶�λ,LONG��,��ʼֵ1
#define de_is_second_locate_eye_infection 6 //�������۶�λ��ĺ��ڼ��㣨���Ƿ���۾����жϣ�,ֻ����һ�����ش򿪵�����£������ز���Ч��LONG��,��ʼֵ1
//���Ҫ��������ʶ������֤�����������ض���Ҫ�򿪡�
//���ֻ�Ǿ����ܶ�ز�׽������������������ùص����Խ�ʡʱ�䡣

//���������أ���������һ�����صĴ򿪲���Ч
#define de_is_second_locate_eye_infection_GETFACE  6001  //LONG��,��ʼֵ0,��Ĥ��ȡ,ֻ����һ�����ش򿪵�����£������ز���Ч��
#define de_is_second_locate_eye_infection_GETFACE_Threshold  6002  //LONG��,��ʼֵ64,����������һ�����صĿ�������Ч
#define de_is_second_locate_eye_infection_CALC_EYEWHITE  6003 ////LONG��,��ʼֵ0 �Ƿ������۰����㣨��Ҫ���л�����֤����˯��⣩

#define de_is_second_locate_eye_infection_FOR_RECOG_OR_GLASS  6004 //LONG��,��ʼֵ1 �Ƿ�Ϊ���ǣ�����ʶ����۾��ж�,�����һ��Ҫ��ģ���ʶ�𣬴����Ҫ��
#define de_is_second_locate_eye_infection_FACE_BORDER  6005        //LONG��,��ʼֵ1 �Ƿ����������߽���ҡ�
#define de_is_second_locate_eye_infection_FACE_NOSE_MONTH  6006    //LONG��,��ʼֵ1 �Ƿ�����������ҡ�


#define de_vertical_angle_limit 7 //LONG��,��ʼֵ60
//���������ƽ����ת�ĽǶȷ�Χ
//һ��أ���ؿ�Ϊ45�ȣ�ʶ��30��(����ʡʱ�����Խ��ʹ�׽�ʣ������Ҳ����ζ�Ŵ���Ҳ��,�ܶ��������Ǵ�׽��)��
//ע���������õ�80�ȣ������������ٶȻ���Ա���

#define de_automode 8  //LONG��,��ʼֵ1
//��������Զ�ģʽ,�Զ�ģʽ���õ�ʱ�������Щ�����ڶ�����ͬʱ����ʱ���н�С����©׽������

#define de_is_run_smalle_face 9  //LONG��,��ʼֵ0
//��С���Ƿ�������"ȫ���ܿ���",		
//�������û���Ҫ�󼰸����80������ʱ�������С����С����������壬���˷Ѵ���ʱ�䣬��Ϊ�����ֻ��50-80�ֶΣ���70�ֵĶ�ֻ�Ǽ�������
//���Դ�ʱС��������йرգ��Լ���ʱ��ռ�á�
//����������ʶ��ʱ�������С����С�����ֻ�ڽ��и������ȵ�ֻ׽�����ļ��ʱ���á�

#define de_is_color_filter 10             //LONG��,��ʼֵ1,��ɫ���� Ĭ���Ǵ򿪵�(Ϊ����)��ֻ�ھưɻ������ص�����ʱ��ɫ���ҵġ�
#define de_color_filter_min_bmp_width 11  //LONG��,��ʼֵ300,����ɫ���˵���Сλͼ����Ϊ����һЩС��λͼ��û�б�Ҫ����ɫ���ˡ�


#define de_face_locate_zdz  12   //FLOAT��,��ֵ 0.0783f  ����һ�β�������ʱ�ģ�
#define de_face_locate_bank_zdz 13//FLOAT��,��ֵ 0.0525f  ���ڶ��β�������ʱ�ģ����Զ��������ģʽ�£���һ��δ�ҵ������������ᷢ���ڶ��Σ�
#define de_face_is_use_bank 131   //LONG,1,�Ƿ������ڶ��β��ң���de_face_locate_bank_zdz���ֵ�����Զ�ģʽ�ͷ��Զ�ģʽ����Ч��
//����������У����������ڲ������ı�Ե��ֵⷧ����ʱһ��ͼ������ȴ�Ҳ������Ը�һ������ҵ��ˣ�������Ԥ��ֵ�����������Եľ�������ֵ��


//������Ҫ��Ե�����zFlagFace��������
//Ŀǰ��������Щ��λ�ľ������������ģ�ֻ�������������������������²��ܱ�֤׼ȷ�ȣ�����ֻ���Ǹ���Լ�ķ�Χ���ơ�
#define de_flagface_draw_eye 14      //LONG��,��ʼֵ1,�Ƿ���
#define de_is_draw_face_border 15    //LONG��,��ʼֵ1,�Ƿ����߽�
#define de_is_draw_nose_month 16     //LONG��,��ʼֵ1,�Ƿ��־����
#define de_draw_nose_month_rely 17   //FLOAT,��ʼֵ0.3f,���������������Ŷ�     
#define de_draw_face_width_rely 18   //FLOAT ,��ʼֵ0.4f,��������(���߽�)��������Ŷ�
#define de_is_have_glass_threshold 19//FLOAT ,��ʼֵ0.65f; �����۾���ǵ�������Ŷ�,�޿��أ�������1.0f�Ͳ�����


//�����ǹ���ģ������
//����ģ�����
		#define de_is_adding_template_scroll  20//LONG��,��ʼֵ1
		//�����ģ��ʱ������ģ���Ƿ񳬹��������͹��������δ�����������������޷��ټ��롣	
				
		//��������˹������£��������������ʱ�����Զ�ɾ���������ģ�壬��ά��ģ���������䡣
		#define de_template_limite_num  21//LONG��,��ʼֵ10,��ÿ�������10��ģ��
		#define de_template_scroll_reserve_num   22//LONG��,��ʼֵ5     
        //���ֶ��ٸ������ģ�壬0�������֣�3�����ʾ�����3��ʼ�ղ����������µ���

		#define de_is_template_recog_adding_update  23//LONG��,��ʼֵ0,
        //ʶ������,���ƶȹ�ָ����ֵ,�Ƿ񽫵�ǰ������Ϊģ�塣
		#define de_is_template_recog_hit_update   24 //LONG��,��ʼֵ0,
        //ʶ���������ƶȹ�ָ����ֵ,�Ƿ��õ�ǰ����Ϊģ���滻���뵱ǰ����������Ǹ�ģ�塣
		//ע�⣺������BOOL���ɿ�����Ա����������Ϊ����ġ�

		#define de_template_scroll_threshold   25 //FLOAT,��ֵ0.85f;������ʶ�����ƶȴ�����85%,���Զ�����ģ��
        //���û����ģ��ʱ�����ֵ�����塣���ֵ�ǳ���Ĭ��ֵ��������Ӧ���û�����ʵ�����������		
        #define de_template_hit_threshold    26  //FLOAT,��ֵ0.85f;������ʶ�����ƶȴ�����85%,���Զ�����ģ��		
		//��ʶ��ʱ�����ƶȴ����˸�����ֵ���ſ��Ը���ģ��
		//END ����ģ�����
 

//������������:

	#define de_1CN_first_filter_reserve_bl  27 //FLOAT,��ֵ0.02f
    //1CN���ڴ������н��й��˺�ı�����  //(2%)
	//���ڴ����������Ĺ�����,ֻ����2%��ģ������һһ��ϸ�ȶԣ����ֵ��С�������٣���������ǧ�˿⣬���˿��У���©ʶ�����ӡ�
   

    //����ʶ��ʱ��������̫С���۾�Ӧ����40pixel, ����˲����߲�����˸
	#define de_check_eye_hd_change_bl  28 //FLOAT,0.45f; //����ġ��仯�ʡ���Ҫ�������ֵ������Ϊգ���ۡ�
	//ʵ�⵱һ����Ƭ����ʱ���仯�ȳ嵽���0.23f.
	//����ġ����ȱ仯�ʡ���Ҫ�������ֵ������Ϊգ���ۡ�
	//��Ӱ��ĶԱȶȣ�������Խ��ʱ��Ϊ�Ӵ����ʶ��Ŀɿ��ԣ�������ߴ�ֵ������ȡ0.55����֮��ȡ0.35
	//������������ۣ�ȴû�л��ʾ�����ճ�ʱ������ʶ��һֱ����0����˵�����ֵӦ��С������������
	//��֮�����ֵ̫С��û����Ҳ������Ϊ�����ۣ����˵���Ƭ��һ�¾��ܹ���������ʶ�𷵻�1����
	//ԭ���ǣ�����Ƥ���������
    #define	de_check_eye_face_change_Threshold 29 //FLOAT,0.33f;//�����Ķ�ֵ�ȶ���Ҫ�������ֵ����
    //�����ȶ��ȣ���ֻ׼�۱䣬�����ط�����ɱ䣬Ŀ�����ڷ�ֹ��Ƭƽ�ƣ������������Ƭ�����ۣ���ƽ��һ�¿ɹ����Ӵ����ֵ��
    //����������δƽ��ȴ����-1����Ҫ�������ֵ��



//��Ϊ����ʶ�����������
ZW_DEF LONG __stdcall zInitCheck(LONG OID,LONG order);
//��ʼ������ʶ���ֳ�������order�������������,һ��ȡ��һ����0


ZW_DEF LONG __stdcall zFrameCheck(LONG OID,CHAR *bmpFileName);//ע�⣬���ﴫ���һ��Ҫ��BMP��ʽ���ļ���������JPG
//֡��⺯��������0ʱ������ִ�У���ʾ��δ���жϳ���������С��0����ʾ����ʶ����ͨ���������ס�����1����ʾ�û��ǻ��塣�б�һ���ۡ�



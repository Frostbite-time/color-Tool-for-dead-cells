#include<opencv2/opencv.hpp>
#include<iostream>
#include<cmath>
#include <string>
#include<io.h>
#include <direct.h>
#include <filesystem>
#include <Windows.h>
using namespace cv;
using namespace std;




/// /// /// 此处用于定义需要使用的全局变量和数据结构等 /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///


string main_window_name = "死亡细胞快速皮肤制作工具";
float main_window_y = (GetSystemMetrics(SM_CYSCREEN)/2);//主要窗口的高  默认为720p，但程序开始之前应该从系统处获取当前屏幕的分辨率
Mat main_window(main_window_y, (main_window_y/9)*16, CV_8UC4, Scalar(255,255,255,255));//主要窗口，也作为基础背景，为白色

//vector<Mat> pic_src(1000);    //本行代码已经弃用

Mat* pic_src = new Mat[1000];//需要处理的图片储存的位置  这里是建立了一个Mat类的指针数组并用new分配了内存
string pic_name[1000]; //储存那些需要处理的图片名称
Mat plat_src;         //色带文件无需大量储存，为保证正确率，每次选择一张新图时候重新读入色带
string plat_name[1000];//储存所有色带文件的名称
string pic_save;//图片的保存位置
string plat_save;//色带的保存位置

string src_s;//需要上色的图片路径(临时路径，每次循环都会被改变)
string src_p;//使用的色带路径(临时路径，每次循环都会被改变)
string path_now;//储存当前路径
int pic_number = 0; //图片的总数，不包含法线等无关图片  这是数量
int pic_name_number = 0;//图片名称的总数，是一个会变化的量  初始化过程中可以当做下标使用，初始化结束之后当数量使用




/// /// /// 此处用于定义一些下方需要使用的函数来加快编程效率 /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///


//舍去小数位小于等于5的数  如果使用自带的round函数会导致坐标计算错误
int round_but_in_five(double becut) {
	int i = becut;
	float o = becut - i;
	int final_number;
	if (o <= 0.5) {
		final_number = becut;
	}
	else
	{
		final_number = becut + 0.5;
	}
	return final_number;

}

//用于计算红色值对应的坐标，以前不需要，自从bossrush过后开始需要
int redcal(int height_re, int re) {
	int re_cal;
	for (int i = 0; i < 256; i++) {
		re_cal = round_but_in_five(((double)255 / height_re) * (0.5 + i));
		if (re_cal == re) {
			return i;
		}
	}

}


//这个函数用来计算绿色值对应坐标  
int greencal(int height_gr, int gr) {
	//更新后的用于计算绿色坐标的函数
	int gr_cal;
	for (int i = 0; i < 256; i++) {
		gr_cal = round_but_in_five(((double)255 / height_gr) * (0.5 + i));
		if (gr_cal == gr) {
			return i;
		}
	}
}

//从坐标反推颜色  x
int redcal_re(int plxt_re, int long_re) {
	return round_but_in_five(((double)255 / long_re) * (0.5 + plxt_re));

}


//从坐标反推颜色  y
int greencal_re(int plyt_gr, int long_gr) {
	return round_but_in_five(((double)255 / long_gr) * (0.5 + plyt_gr));
}


//获取文件的名称
void findfile(string path, string mode, string arr_use[])
{
	int j = 0;
	_finddata_t file;
	intptr_t HANDLE;
	string Onepath = path + mode;
	HANDLE = _findfirst(Onepath.c_str(), &file);
	if (HANDLE == -1L)
	{
		cout << "不能匹配文件夹路径" << endl;
		system("pause");
	}
	do {

		//判断是否有子目录  
		if (file.attrib & _A_SUBDIR)
		{
			//判断是否为"."当前目录，".."上一层目录
			if ((strcmp(file.name, ".") != 0) && (strcmp(file.name, "..") != 0))
			{
				string newPath = path + "\\" + file.name;
				findfile(newPath, mode, arr_use);
			}
		}
		else
		{
			arr_use[j] = file.name;
			j++;
		}
	} while (_findnext(HANDLE, &file) == 0);
	_findclose(HANDLE);
}


//将读取的路径名转换成程序使用
string& replace_all(string& srt, const string& old_value, const string& new_value) {
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
		if ((pos = srt.find(old_value, pos)) != string::npos) {
			srt.replace(pos, old_value.length(), new_value);
		}
		else break;
	}
	return srt;
}


//获取string中第一个数字的位置  用于截取皮肤模式
int getpos_string(string ppos_uesd) {
	if (ppos_uesd == "Frostbite.png")
	{
		cout << endl << endl << "作者b站id为dyfdhfc，感谢使用" << endl << endl;

	}
	for (int h = 0; 1; h++) {
		if (ppos_uesd[h] >= '0' && ppos_uesd[h] <= '9') {
			return h;
		}
		else if (h > ppos_uesd.length())
		{
			break;
		}
	}
	return -1;
}


//接受2个Mat，进行图片上色处理，同时还接受一个字符串，用来决定上色的色带
void pic_get_color(Mat src, Mat gotcolor,string what_plat_is) 
{
	src_p = path_now + "\\palette\\" + what_plat_is;
	plat_src = imread(src_p, -1);
	gotcolor = src.clone();
	for (int i = 0; i < gotcolor.rows; i++) {
		for (int j = 0; j < gotcolor.cols; j++) {
			if (gotcolor.at<Vec4b>(i, j)[3] != 0) {
				int x = redcal(plat_src.cols, gotcolor.at<Vec4b>(i, j)[2]);
				int y = greencal(plat_src.rows, gotcolor.at<Vec4b>(i, j)[1]);
				gotcolor.at<Vec4b>(i, j)[2] = plat_src.at<Vec4b>(y, x)[2];
				gotcolor.at<Vec4b>(i, j)[1] = plat_src.at<Vec4b>(y, x)[1];
				gotcolor.at<Vec4b>(i, j)[0] = plat_src.at<Vec4b>(y, x)[0];
			}
		}
	}
}




/// /// /// 接下来是主程序 /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///



int main()
{   

	/// /// /// 初始化数据结构和一些需要用的变量 /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///
	
	//初始化获取当前目录  然后储存到字符串 path_now 中
	char buffer[250];
	_getcwd(buffer, 250);
	path_now = buffer;
	path_now = replace_all(path_now, "\\", "\\\\");


	//先将所有的图片与色带数组初始化为  there is nothing  这样可以方便后续检测
	for (int i = 0; i < 1000; i++)  
	{
		pic_name[i] = "there is nothing";
		plat_name[i] = "there is nothing";

	}


	//将文件名读取到这些数组内
	findfile(path_now + "\\x64\\Release\\image", "\\*.*", pic_name);
	findfile(path_now + "\\x64\\Release\\palette", "\\*.*", plat_name);


	//获取数组中有效名称的总量即非there is nothing的字符串总量
	for (; pic_name[pic_name_number] != "there is nothing"; pic_name_number++) {}   //经过这道工序的pic_name_number变为了数量而非单纯的下标


	//这一部分将所有图片检测，然后储存到数组 pic_src 中，并自动剔除了所有的法线图和其他不符合皮肤基本定义的图，如图片中没有数字
	for (int k = 0; pic_name[k] != "there is nothing"; k++) {
		
			//开始处理
			//思路，写个检查对应，对应就上色保存，不对应就g
			int pos_of_number = getpos_string(pic_name[k]) - 1;
			
			//如果发现不符合皮肤有数字的基本定义，将名称数组中的最后一个名称提前到该位置，然后重置本次循环，同时使得名称总量减少1
			if (pos_of_number < 0) {          
				pic_name[k] = pic_name[pic_name_number - 1];             
				pic_name[pic_name_number - 1] = "there is nothing";
				pic_name_number = pic_name_number - 1;
				k = k - 1;
				continue;
			}
			string pic_mode = pic_name[k].substr(0, pos_of_number);  //获取皮肤模式
			string test_png = pic_name[k].substr((pic_name[k].find_last_of(".") + 1), (pic_name[k].length() - 1));  //查看最后是否为png
			bool test_png_3 = true;//查看是否为法线

			//这一段检查是否为R5类
			if ((pic_name[k].find_last_of("R") >= 0) && (pic_name[k].find_last_of("R") < (pic_name[k].length()))) {
				if (pic_name[k].at(pic_name[k].find_last_of("R") + 2) >= '0' && pic_name[k].at(pic_name[k].find_last_of("R") + 2) <= '9') {
					pic_mode = pic_name[k].substr(0, getpos_string(pic_name[k]));
				}
			}

			//这一段实现法线检查
			if ((pic_name[k].find_last_of("_") >= 0) && (pic_name[k].find_last_of("_") < (pic_name[k].length()))) {
				if (pic_name[k][pic_name[k].find_last_of("_") + 1] == 'n')
				{
					test_png_3 = false; //标记为false的就是下划线
					//如果发现不符合皮肤有数字的基本定义，将名称数组中的最后一个名称提前到该位置，然后重置本次循环，同时使得名称总量减少1
					pic_name[k] = pic_name[pic_name_number - 1];
					pic_name[pic_name_number - 1] = "there is nothing";
					pic_name_number = pic_name_number - 1;
					k = k - 1;
					continue;   //如果发现函数是发现是法线，直接打破循环，寻找下一张图片
					
				}
				else
				{
					test_png_3 = true; //否则仍然为true
				}
			}

			//这一段判断文件是否为png以及是否发现，如果不符合就进行下一次循环，不影响使用
			if ((test_png == "png") && (test_png_3))
			{
				if (pic_name[k] == "there is nothing")      //当读取到名称数组为空的时候跳出本段循环，进入下一步处理（实际上不需要这个）
				{											//因为循环体开始已经写了检测
					break;
				}
				src_s = path_now + "\\x64\\Release\\image\\" + pic_name[k];  //将名称绑定到临时路径上
				pic_src[k] = imread(src_s, -1);                //利用临时路径把图片读取到Mat数组内
				pic_number++;  //计数图片总量
			}
			else
			{	//这一部分用于解决非png文件对数组内存空间混乱的处理
				pic_name[k] = pic_name[pic_name_number - 1];
				pic_name[pic_name_number - 1] = "there is nothing";
				pic_name_number = pic_name_number - 1;
				k = k - 1;
				continue;
			}
		
	}


	//绘制开始之前，将图片进行缩放，用于制作预览图，缩放的大小比例应该使得图片的高为窗口大小的五分之一
	Mat* pic_src_small = new Mat[pic_number];  //这个数组用来储存缩放后的图像
	//把所有图像按比例缩放到主窗口的四分之一
	for (int k = 0; k < pic_number; k++) 
	{		
		resize(pic_src[k], pic_src_small[k], Size(), ((double)main_window_y / 4) / pic_src[k].rows, ((double)main_window_y / 4) / pic_src[k].rows);
	
	}  

	/// /// /// 下面是窗口循环体开始 /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ////// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// /// ///

    //绘制窗口     
    while (1){
		namedWindow(main_window_name, WINDOW_KEEPRATIO);
		imshow(main_window_name, main_window);

		//每日更新计划
		//下一步是要将缩放图拼接在一起并且将它设置为按钮，然后添加滑块，分别用来浏览所有的缩放图、浏览大图、以及浏览上色后的大图、以及对大图进行
		//缩放观察等

		




		/*for (int k = 0; k < pic_number; k++) {
			imshow(pic_name[k], pic_src_small[k]);
		
		}*/
		


		waitKey(10);//以大约10毫秒的速度刷新一次窗口，约为90hz的刷新率

		if (getWindowProperty(main_window_name, WND_PROP_AUTOSIZE) != 0) //这个if使得程序可以在按下x的时候关闭
		{
		   break;
		}

    }
    return 0;//我也不懂为什么我之前没写return 0还能过编译这就是release吗？   原来我不写也会自动给我加上



}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

#include<opencv2/opencv.hpp>
#include<iostream>
#include<cmath>
#include <string>
#include<io.h>
#include <direct.h>
#include <filesystem>
using namespace cv;
using namespace std;
std::string src_s;//需要上色的图片路径
std::string src_p;//使用的色带路径
string src_save;//保存的位置
Mat src(5000, 5000, CV_8UC3, Scalar(0, 255, 0));  //先行分配空间防止内存冲突   需要上色的图片   我也不知道这样做是否正确，但是这貌似有点玄学的因素了
Mat plate(100, 100, CV_8UC3, Scalar(0, 255, 0));   //需要上色的色带图

const int size_pic_arr = 1000;   //最多可以处理的图片总数


//储存像素点的颜色信息和位置,用于去色阶段的处理
struct color_pot 
{
	int red_d;//红色通道
	int green_d;//绿色通道
	int blue_d;//蓝色通道
	int alp;//透明通道
};



//将读取的路径名转换成程序使用     这一段貌似是从网上扒下来的代码
string& replace_all(string& srt, const string& old_value, const string& new_value) {
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
		if ((pos = srt.find(old_value, pos)) != string::npos) {
			srt.replace(pos, old_value.length(), new_value);
		}
		else break;
	}
	return srt;
}


//舍去小数位小于等于5的数  如果使用自带的round函数会导致坐标计算错误
int round_but_in_five(double becut) {
	int i = becut;   //将double类型参数传递到int中后会自动舍去小数点后数据
	float o = becut - i;   //这时用原来的double类型数减去int类型，就可以得到小数点后的数字
	int final_number;   //存储最终数字
	if (o <= 0.5) {   //比较大小，以确定四舍五入的方向
		final_number = becut;
	}
	else
	{
		final_number = becut + 0.5;
	}
	return final_number;

}

//读取色带文件和图片文件   (用于单个图片的上色)
void readfiles_only() {
	
}

//用于计算红色值对应的坐标，以前不需要，自从bossrush过后开始需要添加为x轴非256个长度的调色板添加支持
int redcal(int height_re, int re) {
	int re_cal;   //定义一个int值用于储存计算得到的红色数值
	for (int i = 0; i < 256; i++) {   //循环计算红色数值，当计算得到的红色数值等于当前真实的红色数值跳出循环，返回i  这里的i表示红色的坐标
		re_cal = round_but_in_five(((double)255 / height_re) * (0.5 + i));   //  （255 / 色带总长（或总宽）） * （0.5 + 当前坐标）
		if (re_cal == re) {													 //   得出的结果进行四舍五入即为去色图的红色值  这里四
			return i;														 //   舍五入需要大于0.5才能进位，等于0.5不能进位
		}
	}
}


//这个函数用来计算绿色值对应坐标  
int greencal(int height_gr,int gr) {   //同上
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
int redcal_re(int plxt_re,int long_re) {   //原理同上，这里因为当前坐标已知，所以不需要循环
	return round_but_in_five(((double)255 / long_re) * (0.5 + plxt_re));

}


//从坐标反推颜色  y
int greencal_re(int plyt_gr, int long_gr) {   //同上
	return round_but_in_five(((double)255 / long_gr) * (0.5 + plyt_gr));
}


//获取一个目录下所有文件的名称并存储到数组中
void findfile(string path, string mode,string arr_use[])
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


//获取string中第一个数字的位置  用于截取皮肤模式
int getpos_string(string ppos_uesd) {
	if (ppos_uesd == "Frostbite.png")  //彩蛋
	{
		cout << endl << endl << "作者b站id为dyfdhfc，感谢使用" << endl << endl;

	}
	for (int h = 0; 1; h++) {   //这里的h是数字在字符串中的位置，遵从数组原理。 
		if (ppos_uesd[h] >= '0' && ppos_uesd[h] <= '9') {  //第一个if判断字符串中的第h-1个字符是否是数字，如果是，返回当前h值。
			return h;
		}
		else if(h>ppos_uesd.length())  //如果h的值大于字符串的长度，那么结束这个循环，返回值-1在会导致后面的调用中有一个小于0的判定为真，然后重置一个循环体
		{
			break;
		}
	}
	return -1;
}

//检查字符串数组中是否只剩下一种字符串     //一个简单的遍历，无需解释
bool is_there_only_one_string(string only_one_string[],int the_long,string what_has) {
	for (int i = 0; i < the_long; i++) {
		if (only_one_string[i] != what_has) {
			return false;
		
		}
	
	}
	return true;

}





//主程序运行
int main()
{	
	//初始化获取当前目录  当前目录即程序运行目录
	char buffer[250];
	_getcwd(buffer, 250);
	string path_now = buffer;
	path_now = replace_all(path_now, "\\", "\\\\");  //将路径转换为程序可以读取的路径


	string got_block;  //定义一个string类 用于储存后面你键盘的输入
	cout << "请选择上色模式" << endl;
	cout << "1.单张图片上色" << endl;
	cout << "2.批量上色" << endl;
	cout << "3.自动去色" << endl;
	cout << "4.阴影添加" << endl;
	cout << "或者按任意键退出" << endl;
	getline(cin, got_block);   //获取当前输入
	while ((got_block == "1") || (got_block == "2")|| (got_block == "3")|| (got_block == "4"))  //简单的循环判断，如果是这4个键值中的任何一个，继续循环
	{	



		
		/// 路径上色  //////
		

		if ((got_block == "1")) 
		{
			

			cout << "请输入需要上色的图片路径" << endl;
			getline(cin, src_s);
			src_s = replace_all(src_s, "\\", "\\\\");
			cout << "请输入使用的色带路径" << endl;
			getline(cin, src_p);
			src_p = replace_all(src_p, "\\", "\\\\");
			cout << "请输入完成后保存的位置" << endl;
			getline(cin, src_save);
			src_save = replace_all(src_save, "\\", "\\\\");
			src_save = src_save + "\\\\";
			src = imread(src_s, -1);
			Mat plate_ee = imread(src_p, -1);  //将色带文件读取到plate_ee中
			

			if (src.empty())      //读取错误时候输出
			{
				printf("无法读取图像\n");
				return -1;
			}
			
			
			if (plate_ee.type() != 24) {   //检测图片是否为CV_8UC4，即是每个像素点是否拥有4个通道并且每个通道占用8bit
				cvtColor(plate_ee, plate, COLOR_BGR2BGRA);  //如果不是，为其添加透明通道，并输出到plate中
			}
			else {
				plate = plate_ee;  //如果是，那么直接将plate_ee储存到plate中
			}

			/// /// //// /// /// /// /// /// /// /// 
			//逐个替换像素
			for (int i = 0; i < src.rows; i++) {
				for (int j = 0; j < src.cols; j++) {
					if (src.at<Vec4b>(i, j)[3] != 0) {   //检测是否为透明像素，如果是，那么跳过
						int x = redcal(plate.cols, src.at<Vec4b>(i, j)[2]);  //如果不是，计算其在色带上对应的x，y坐标，并保存
						int y = greencal(plate.rows, src.at<Vec4b>(i, j)[1]);
						src.at<Vec4b>(i, j)[2] = plate.at<Vec4b>(y, x)[2];  //将图片的颜色替换为色带上对应坐标的颜色
						src.at<Vec4b>(i, j)[1] = plate.at<Vec4b>(y, x)[1];
						src.at<Vec4b>(i, j)[0] = plate.at<Vec4b>(y, x)[0];
					}
				}
			}


			//获取图片名称用于保存
			string pic_name;
			string::size_type iPos = src_s.find_last_of('\\') + 1;
			pic_name = src_s.substr(iPos, src_s.length() - iPos);
			src_save = src_save + pic_name;

			//保存图片并准备开始下一次循环
			imwrite(src_save, src);
			cout << endl << "上色完成" << endl;
			cout << endl << "请选择上色模式" << endl;
			cout << "1.单张图片上色" << endl;
			cout << "2.批量上色" << endl;
			cout << "3.自动去色" << endl;
			cout << "或者按任意键退出" << endl;
			getline(cin, got_block);
		}








		/// 自动批量上色 //////	   //该段的注释与阴影添加的注释极其相似，参考阴影添加的注释


		if (got_block == "2") 
		{
			
	
			
			string pic_name[size_pic_arr]; //图片的名称 数组  最多可储存1000个
			string plat_name[size_pic_arr]; //色带的名称 数组
			//初始化这些数组以免造成不必要的错误
			for (int i = 0; i < size_pic_arr; i++) 
			{
				pic_name[i] = "there is nothing";
				plat_name[i] = "there is nothing";
	
			}
			///////将文件名读取到这些数组内

			findfile(path_now+"\\image", "\\*.*",pic_name);
			findfile(path_now + "\\palette", "\\*.*", plat_name);//因为加了此段函数，测试时候必须在非调试模式


			///////开始对函数名称进行处理并对应上色
			for (int k = 0; pic_name[k] != "there is nothing"; k++) {
				for (int l = 0; plat_name[l] != "there is nothing"; l++) {
					//开始处理
					//思路，写个检查对应，对应就上色保存，不对应就g
					int pos_of_number = getpos_string(pic_name[k]) - 1;
					if (pos_of_number < 0) {
						continue;
					}
					string pic_mode = pic_name[k].substr(0, pos_of_number);  //获取皮肤模式
					string plat_mode = plat_name[l].substr(0, plat_name[l].find('_') - 1);    //获取色带模式
					string test_png = pic_name[k].substr((pic_name[k].find_last_of(".") + 1),(pic_name[k].length()-1));  //查看最后是否为png
					string test_png_2 = plat_name[l].substr((plat_name[l].find_last_of(".") + 1),(plat_name[l].length()-1));   //查看最后是否为png
					bool test_png_3 = true;//查看是否为法线



					//这一段检查是否为R5类
					if ((pic_name[k].find_last_of("R") >= 0) && (pic_name[k].find_last_of("R") < (pic_name[k].length()))) {
						if (pic_name[k].at(pic_name[k].find_last_of("R")+2) >= '0' && pic_name[k].at(pic_name[k].find_last_of("R")+2) <= '9') {
							pic_mode = pic_name[k].substr(0, getpos_string(pic_name[k]));
						
						}


					}




					//这一段实现法线检查
					if ((pic_name[k].find_last_of("_")>=0)&&(pic_name[k].find_last_of("_") < (pic_name[k].length()))) {
						if (pic_name[k][pic_name[k].find_last_of("_") + 1] == 'n')
						{
							test_png_3 = false; //标记为false的就是下划线
							continue;   //如果发现函数是发现是法线，直接打破循环，寻找下一张图片
						}
						else
						{
							test_png_3 = true; //否则仍然为true
						}
						
					
					}


					//这一段判断文件是否为png以及是否发现，如果不符合就进行下一次循环，不影响使用
					if ((test_png == "png") && (test_png_2 == "png")&&(test_png_3))
					{
						
						
						

					//接下来如果图片与色带模式相同，读取并上色
						if (pic_mode == plat_mode) {
							src_s = path_now + "\\image\\" + pic_name[k];
							src_p = path_now + "\\palette\\" + plat_name[l];
							src = imread(src_s, -1);
							Mat plate_e = imread(src_p, -1);
							if (src.empty())      //读取错误时候输出
							{
								printf("无法读取图像\n");
								return -1;
							}
							if (plate_e.type() != 24) {
								cvtColor(plate_e, plate, COLOR_BGR2BGRA);
							}
							else {
								plate = plate_e;
							}

							/// /// //// /// /// /// /// /// /// /// 
							//逐个替换像素
							for (int i = 0; i < src.rows; i++) {
								for (int j = 0; j < src.cols; j++) {
									if (src.at<Vec4b>(i, j)[3] != 0) {
										int x = redcal(plate.cols, src.at<Vec4b>(i, j)[2]);
										int y = greencal(plate.rows, src.at<Vec4b>(i, j)[1]);
										src.at<Vec4b>(i, j)[2] = plate.at<Vec4b>(y, x)[2];
										src.at<Vec4b>(i, j)[1] = plate.at<Vec4b>(y, x)[1];
										src.at<Vec4b>(i, j)[0] = plate.at<Vec4b>(y, x)[0];
									}
								}
							}

							
							string save_path = path_now +"\\completion\\" + plat_name[l].substr(0, plat_name[l].find_last_of('_')) + "\\";
							if (_access(save_path.c_str(), 0) == -1)	//如果文件夹不存在
								_mkdir(save_path.c_str());
							//保存图片并准备开始下一次循环
							imwrite(save_path + pic_name[k], src);
							

							
						}

						
					}
					else
					{
						continue;
					}

					
					



				}
			
			}
			





			////////////////////////////////////
			cout << endl << "上色完成" << endl;
			cout << endl << "请选择上色模式" << endl;
			cout << "1.单张图片上色" << endl;
			cout << "2.批量上色" << endl;
			cout << "3.自动去色" << endl;
			cout << "4.阴影添加" << endl;
			cout << "或者按任意键退出" << endl;
			getline(cin, got_block);

		}






		/// 自动批量去色 //////
		if ((got_block == "3"))
		{
			//把所有图片名称读取到数组
			string model;//存储你将会生成的色带的色带名称
			cout << "请为即将生成的色带取名，如果需要覆盖原版色带只需要取重复名称即可(如不能理解请看说明文档)" << endl;
			getline(cin, model);


			string pic_name[size_pic_arr];//声明图片名称的数组
			//初始化这些数组以免造成不必要的错误
			for (int i = 0; i < size_pic_arr; i++)
			{
				pic_name[i] = "there is nothing";    //数组内所有元素都叫做there is nothing这种标志性的字符串从而用来判断该位置还有没有储存图片名称

			}
			findfile(path_now + "\\image_decoloured", "\\*.*", pic_name);//读取图片名称到数组中



			/// ////////////////////////////////////////////////////////////////////////////////////////////////
				

			//这部分作为专门获取模式名称的部分
			string pic_mode_name[100][10];    //这里是一个2维数组，第一维代表皮肤模式，第二维代表是该模式皮肤的第n张图片
											  //这里的数组可以存储100个不同模型的皮肤，并为每种皮肤存储10种图片，一般情况已经够用
											  //这里实际存储的是图片的名称而非图片本身，为了节省内存空间而做的努力	
			//将其初始化					   
			for (int i = 0; i < 100; i++)
			{
				for (int j = 0; j < 10; j++) {
					pic_mode_name[i][j] = "there is nothing";  //同上
				}

			}

			int k_1 = 0;//k_1 + 1就是模式的总数
			int k_2 = 0;//k_2 + 1就是每个模式下图片的总数
			string pic_mode_2; //为了后面预处理图片名称而声明的string变量   意为，上一个处理的图片名称
			for (int k = 0; k < size_pic_arr;k++) {  //这一段循环都是用来预处理图片名称的。这也是为什么自动去色可以从一堆图片名称中分辨出来不同的皮肤
			
				//检查是否为一个皮肤文件  皮肤文件必然带有数字
				int pos_of_number = getpos_string(pic_name[k]) - 1;
				if (pos_of_number < 0) {
					continue;
				}

				string pic_mode = pic_name[k].substr(0, pos_of_number);  //获取皮肤模式
				string test_png = pic_name[k].substr((pic_name[k].find_last_of(".") + 1), (pic_name[k].length() - 1));  //查看最后是否为png
				bool test_png_3 = true;//查看是否为法线



				//这一段检查是否为R5类   截取皮肤模式时候使用的是数字，如beheaded1.png 截取后为beheaded 本无问题。但beheadedAssassinR51.png 被截取后会成为beheadedAssassinR 而不是正确的beheadedAssassinR5
				if ((pic_name[k].find_last_of("R") >= 0) && (pic_name[k].find_last_of("R") < (pic_name[k].length()))) {
					if (pic_name[k].at(pic_name[k].find_last_of("R") + 2) >= '0' && pic_name[k].at(pic_name[k].find_last_of("R") + 2) <= '9') {
						pic_mode = pic_name[k].substr(0, getpos_string(pic_name[k]));

					}
					//原理非常简单，就是单纯检测字符串范围内有没有R，如果有，就将截取位置向R后调即可

				}



				//这一段实现法线检查    如果名称中有 _n 就判定为法线，跳过，因此test_png3实际没有意义。它只有能通过检测才能被后续判定捕捉，而它如果要通过就必须是true
				if ((pic_name[k].find_last_of("_") >= 0) && (pic_name[k].find_last_of("_") < (pic_name[k].length()))) {
					if (pic_name[k][pic_name[k].find_last_of("_") + 1] == 'n')
					{
						test_png_3 = false;
						continue;
					}
					else
					{
						test_png_3 = true;
					}


				}
				//至此，检查预备工作结束

				//这一段判断文件是否为png以及是否为法线，如果不符合就进行下一次循环，不影响使用
				if ((test_png == "png") && (test_png_3))
				{
					if (k == 0) {
						pic_mode_2 = pic_mode;  //k等于0的情况说明是第一张图片，不存在上一张图片，因此将pic_mode赋值到pic_mode_2。 这一步相当于一个预备动作，消除没有第一张图片名称与第二张图片名称对比造成的误差，供下一个if使用
					}
					if (pic_mode_2 == pic_mode) {   //如果上一张图片的模式与这一张相同，就说明目前正在同一种模式的皮肤图片，将其记录在对应位置之后只要将数组的第二维上调1即可。并且由于2张图片模式相同，所以并没有更新pic_mode_2的必要
						pic_mode_name[k_1][k_2] = pic_name[k];
						k_2++;
						pic_name[k] = "there is nothing";  //由于图片名称已经分类式的存储到了另一个位置，此时就可以将这个已经无用的元素标记为 空（这里指将其储存的字符串改为there is nothing）
					}
					else //如果上一张图片模式与这一张不同，说明这时在处理另一中模型的图片了。先将二维重置为0，再将第一维上调1。然后就是与上一个if类似的处理过程了。
					{
						k_2 = 0;
						k_1++;
						pic_mode_2 = pic_mode;  //由于此时2个模式有差别，所以在那些if判定结束过后要重新赋值pic_mode_2。 这里解释一下，虽然pic_mode不是数组，但是它是由一个数组为其赋值。所以每次循环其实都会更新
						pic_mode_name[k_1][k_2] = pic_name[k];
						pic_name[k] = "there is nothing";
						k_2++;
					}
					
				}
				// 检查何时退出的函数
				if (is_there_only_one_string(pic_name, size_pic_arr, "there is nothing")) {  //当最初存储图片名称的数组为空，说明对图片名称的预处理结束。
					break;

				}

				//这里说明一下为什么要这么一段繁琐的处理过程。
				//因为要生成色带的话需要将一张皮肤所拥有的的所有颜色全部整合到一个色带中。而每个皮肤拥有不止1张图片，比如beheaded0.png beheaded1.png等
				//这时有2种方法，一是像我这样分类，然后直接遍历数组名称；二是用2个数组，一个数组记录名称，另一个数组记录已经被处理过的图片的名称以防止重复处理并且通过记录皮肤模式来说明目前正在处理哪个皮肤。
				//我这边使用了前者。没有经过对比测试，但是经过单方面测试这样分类名称的方式已经有足够的速度
			}
			/// /// /// /// /// /// /// /// ///




			/// 循环去色，直到模式耗尽   即二维数组的第一维超出了范围

			for (int i_1 = 0; i_1 < k_1 + 1; i_1++)  //i_1即为当前处理的模式， k_1即为模式总数
			{
				color_pot colors[5000];//一个巨大的结构数组，用于储存临时色带
				int pot_ttypes = 0;  //记录目前一共获得了多少种颜色

				//初始化这个巨大的数组，以便调用
				for (long i = 0; i < 5000; i++) {
					colors[i].red_d = 0;
					colors[i].green_d = 0;
					colors[i].blue_d = 0;
					colors[i].alp = 0;
				}

				
				cout << "正在处理色带，耗时较久，如果过久无反应，尝试按下回车" << endl;  //未知问题，但是按下回车就会好。
				//首先读取同一个模式下所有的图片的颜色总和
				for (int j_1 = 0; pic_mode_name[i_1][j_1]!= "there is nothing"; j_1++)
				{
					src_s = path_now + "\\image_decoloured\\" + pic_mode_name[i_1][j_1];  //初始化需要去色的图片的路径
					cout << src_s << endl;  //输出它，以便告诉你，处理正在进行，程序没有卡死
					src = imread(src_s, -1); //读入需要去色的图片
					for (int i = 0; i < src.rows; i++)   //开始遍历
					{	
						for (int j = 0; j < src.cols; j++) {
							if (src.at<Vec4b>(i, j)[3] != 0) {  //忽略透明部分以增加速度
								//初始化，将第一个颜色读入
								if (pot_ttypes == 0) {   //如果当前颜色种类为0，先储存第一个颜色
									colors[pot_ttypes].red_d = src.at<Vec4b>(i, j)[2];
									colors[pot_ttypes].green_d = src.at<Vec4b>(i, j)[1];
									colors[pot_ttypes].blue_d = src.at<Vec4b>(i, j)[0];
									colors[pot_ttypes].alp = src.at<Vec4b>(i, j)[3];
									pot_ttypes++;

								}
								else
								{


									bool not_find_same_color = true;   //初始化一个量，用于后续判断是否为相同的颜色   为真则说明色带中没有和图片相同的颜色

									

									//每个非透明的像素点都会遍历一整张图片以保证没有遗漏
									for (int i = 0; i < src.rows; i++) {    
										for (int j = 0; j < src.cols; j++) {
											if (src.at<Vec4b>(i, j)[3] != 0) {  //忽略透明像素
												if ((colors[pot_ttypes-1].red_d == src.at<Vec4b>(i, j)[2]) && (colors[pot_ttypes-1].green_d == src.at<Vec4b>(i, j)[1]) && (colors[pot_ttypes-1].blue_d == src.at<Vec4b>(i, j)[0])) {
													not_find_same_color = false;     //首先检测上一个存储的颜色是否和这个颜色相同。如果是，那么直接设置为假，并且将图片的这部分设为透明以加快下一次循环到此处的速度
													src.at<Vec4b>(i, j)[3] = 0;      //事实证明这个设置为透明非常有用，将速度从5分钟提升到了50秒

												}
												else {     //如果上一个存储的颜色不同的话，再检测已经存储的所有颜色中是否有相同的。以免遗漏
													for (int times_i = 0; times_i < pot_ttypes; times_i++) {
														if ((colors[times_i].red_d == src.at<Vec4b>(i, j)[2]) && (colors[times_i].green_d == src.at<Vec4b>(i, j)[1]) && (colors[times_i].blue_d == src.at<Vec4b>(i, j)[0])) {
															not_find_same_color = false;
															src.at<Vec4b>(i, j)[3] = 0;//将相同的颜色设置为透明，增加下一次循环的速度
															continue;//如果真的一个相同的颜色都没有，就说明这一种颜色已经被记录完毕，跳转到下一个有颜色的像素点再次开始循环。这其实是一个非常好的算法，由于相同的颜色已经被设置为透明，一种颜色最多只需要2次遍历就能结束，第一次将其储存到颜色列表中，第二次遍历就会将剩下所有的该颜色设置为透明
														}
													}

												}
											}

										}
									}
									




									if (not_find_same_color)   //如果真的找到了不同的颜色，将这个不同的颜色存储到颜色列表，并且增加颜色的种类
									{
										
										pot_ttypes++;
										colors[pot_ttypes - 1].red_d = src.at<Vec4b>(i, j)[2];
										colors[pot_ttypes - 1].green_d = src.at<Vec4b>(i, j)[1];
										colors[pot_ttypes - 1].blue_d = src.at<Vec4b>(i, j)[0];
										colors[pot_ttypes - 1].alp = src.at<Vec4b>(i, j)[3];

									}
									
								}




							}
						}
					}

					
				
				}


				
				
				//开始生成同一个模式下的色带
				//接下来计算生成的色带大小
				int plate_rows = 1;//用于储存生成色带的高
				int plate_cols = 1;//用于储存生成色带的长
				plate_rows = (pot_ttypes / 256) + 1;  //计算生成色带的高并赋值   由于pot_ttypes是一个int类型，所以最后需要+1以补充被忽略的部分
				if (pot_ttypes >= 256) {   //计算生成色带的长并赋值。  如果颜色种类大于等于256，那么长度即等于256。否则等于颜色种类数量，这也是记录颜色种类数量的意义
					plate_cols = 256;
				}
				else {
					plate_cols = pot_ttypes;
				}
				Mat plate_created(plate_rows, plate_cols, CV_8UC4, Scalar(0, 0, 0, 0));   //初始化色带图 其所有像素均为R=0,G=0,B=0,A=0，以待后续写入 

				int pot_pos_zero = 0;//接下来的步骤很简单，就是将之前的颜色列表中存储的所有颜色一个个写入色带图中。pot_pos_zero用于指出第几个颜色将被写入
				for (int m = 0; m < plate_created.rows; m++) {
					for (int n = 0; n < plate_created.cols; n++) {
						plate_created.at<Vec4b>(m, n)[0] = colors[pot_pos_zero].blue_d;
						plate_created.at<Vec4b>(m, n)[1] = colors[pot_pos_zero].green_d;
						plate_created.at<Vec4b>(m, n)[2] = colors[pot_pos_zero].red_d;
						plate_created.at<Vec4b>(m, n)[3] = colors[pot_pos_zero].alp;
						pot_pos_zero++;
					}
				}




				//检查是否为一个图片文件   这些代码与 批量上色 和 添加阴影 作用基本一致，可以去参考他们的注释
				int pos_of_number = getpos_string(pic_mode_name[i_1][0]) - 1;
				if (pos_of_number < 0) {
					continue;
				}

				string pic_mode = pic_mode_name[i_1][0].substr(0, pos_of_number+1);  //获取皮肤模式
				//这一段检查文件是否在名字中包含R5
				if ((pic_mode_name[i_1][0].find_last_of("R") >= 0) && (pic_mode_name[i_1][0].find_last_of("R") < (pic_mode_name[i_1][0].length()))) {
					if (pic_mode_name[i_1][0].at(pic_mode_name[i_1][0].find_last_of("R") + 2) >= '0' && pic_mode_name[i_1][0].at(pic_mode_name[i_1][0].find_last_of("R") + 2) <= '9') {
						pic_mode = pic_mode_name[i_1][0].substr(0, getpos_string(pic_mode_name[i_1][0])+1);

					}


				}

				

				string save_plate_path = path_now + "\\completion\\palette\\";
				if (_access(save_plate_path.c_str(), 0) == -1)	//如果文件夹不存在
					_mkdir(save_plate_path.c_str());//创建该文件夹
				//保存图片并准备开始下一次循环
				imwrite(save_plate_path + pic_mode + "_" + model + "_s.png", plate_created);  //将色带文件保存

				int how_i_stop = 0;
				cout << "正在生成去色图，如果过久无反应，尝试按下回车" << endl;
				////最后一步，将该模式下的所有图片去色，输出
				for (int tt_1 = 0; pic_mode_name[i_1][tt_1] != "there is nothing"; tt_1++) 
				{
					//开始去色处理
					src_s = path_now + "\\image_decoloured\\" + pic_mode_name[i_1][tt_1];
					
					cout << pic_mode_name[i_1][tt_1] << endl;
					Mat src_zero = imread(src_s, -1);//获取一张新的图片，之前那张被加满了透明度标记的图片已经废了

					//对图像去色处理并利用透明度标记
					for (int iny = 0; iny < plate_created.rows; iny++) {     //遍历色带文件
						for (int inx = 0; inx < plate_created.cols; inx++) {
							if (plate_created.at<Vec4b>(iny, inx)[3] != 0) {  //如果色带文件中，该坐标透明度不为0则接着遍历图片文件
								
								
								for (int iyx = 0; iyx < src_zero.rows; iyx++) {
									for (int iyy = 0; iyy < src_zero.cols; iyy++) {
										if (src_zero.at<Vec4b>(iyx, iyy)[3] != 0) {  //忽略透明像素
											if (src_zero.at<Vec4b>(iyx, iyy) == plate_created.at<Vec4b>(iny, inx))//如果图片像素与色带像素颜色以及透明度一样则进行处理
											{
												src_zero.at<Vec4b>(iyx, iyy)[0] = 0;//将蓝色值设为0
												src_zero.at<Vec4b>(iyx, iyy)[1] = greencal_re(iny, plate_created.rows);//设置红色坐标
												src_zero.at<Vec4b>(iyx, iyy)[2] = redcal_re(inx, plate_created.cols);//设置红色坐标
												src_zero.at<Vec4b>(iyx, iyy)[3] = 254;//将透明度从255设为254，这样下一次遍历时会忽略图片文件中这个已经被处理的像素，因为色带中透明度要么为0要么为255




											}
										}

									}
								}
							}
						}
					}

					//还原透明度标记   遍历图片，将之前所有被设为254的像素全部还原回255以便游戏读取（不知道会不会对游戏读取有影响，但是目前来看，正常的都是255）
					for (int iyyx = 0; iyyx < src_zero.rows; iyyx++) {
						for (int iyyy = 0; iyyy < src_zero.cols; iyyy++) {
							if (src_zero.at<Vec4b>(iyyx, iyyy)[3] != 0)
							{
								src_zero.at<Vec4b>(iyyx, iyyy)[3] = 255;
							}

						}
					}

					//准备保存


					save_plate_path = path_now + "\\completion\\pictures_no_color\\";
					if (_access(save_plate_path.c_str(), 0) == -1)	//如果文件夹不存在
						_mkdir(save_plate_path.c_str());//创建文件夹
					//保存图片并准备开始下一次循环
					imwrite(save_plate_path + pic_mode_name[i_1][tt_1], src_zero);
					
					
				
				
				}

				//////
			
			}


			/// /// /// /// /// /// /// /// ///

			///////////////////////////////////////////////////////////////////////////////////////

			

			


			//这段区域内被注释掉的代码是被废弃的

			
		






















				/*
				//开始处理
				//思路，写个检查对应，对应就上色保存，不对应就g
				int pos_of_number = getpos_string(pic_name[k]) - 1;
				if (pos_of_number < 0) {
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
						test_png_3 = false;
						continue;
					}
					else
					{
						test_png_3 = true;
					}


				}

				

				//这一段判断文件是否为png以及是否为法线，如果不符合就进行下一次循环，不影响使用
				if ((test_png == "png")&& (test_png_3))
				{
					src_s = path_now + "\\x64\\Release\\image_decoloured\\" + pic_name[k];
					src = imread(src_s, -1);

					color_pot colors[5000];//一个巨大的结构数组，用于储存临时色带
					int pot_ttypes = 0;

					//初始化这个巨大的数组，以便调用
					for (long i = 0; i < 5000; i++) {
						colors[i].red_d = 0;
						colors[i].green_d = 0;
						colors[i].blue_d = 0;
						colors[i].alp = 0;
					}
					
					for (int i = 0; i < src.rows; i++) 
					{
						for (int j = 0; j < src.cols; j++) {
							if (src.at<Vec4b>(i, j)[3] != 0) {
								//初始化，将第一个颜色读入
								if (pot_ttypes == 0) {
									colors[pot_ttypes].red_d = src.at<Vec4b>(i, j)[2];
									colors[pot_ttypes].green_d = src.at<Vec4b>(i, j)[1];
									colors[pot_ttypes].blue_d = src.at<Vec4b>(i, j)[0];
									colors[pot_ttypes].alp = src.at<Vec4b>(i, j)[3];
									pot_ttypes++;
									
								}
								else
								{


									bool not_find_same_color = true;

									//寻找图片中是否有其他颜色一样的色块
									for (int i = 0; i < src.rows; i++) {
										for (int j = 0; j < src.cols; j++) {
											if (src.at<Vec4b>(i, j)[3] != 0) {
												if ((colors[pot_ttypes - 1].red_d == src.at<Vec4b>(i, j)[2]) && (colors[pot_ttypes - 1].green_d == src.at<Vec4b>(i, j)[1]) && (colors[pot_ttypes - 1].blue_d == src.at<Vec4b>(i, j)[0])) {
													not_find_same_color = false;	
													src.at<Vec4b>(i, j)[3] = 0;
												}
											}

										}
									}

									if (not_find_same_color)
									{
										pot_ttypes++;
										colors[pot_ttypes-1].red_d = src.at<Vec4b>(i, j)[2];
										colors[pot_ttypes-1].green_d = src.at<Vec4b>(i, j)[1];
										colors[pot_ttypes-1].blue_d = src.at<Vec4b>(i, j)[0];
										colors[pot_ttypes - 1].alp = src.at<Vec4b>(i, j)[3];


										



									}
								}

								
								

							}
						}
					}

					
					
					
					//接下来计算生成的色带大小
					int plate_rows = 1;//生成色带的高
					int plate_cols = 1;//生成色带的长
					plate_rows = (pot_ttypes / 256) + 1;
					if (pot_ttypes >= 256) {
						plate_cols = 256;
					}
					else {
						plate_cols = pot_ttypes;
					}
					Mat plate_created(plate_rows, plate_cols, CV_8UC4, Scalar(0, 0, 0,0));   //需要上色的色带图

					int pot_pos_zero = 0;
					for (int m = 0; m < plate_created.cols; m++) {
						for (int n = 0; n < plate_created.rows; n++) {
								plate_created.at<Vec4b>(n, m)[0] = colors[pot_pos_zero].blue_d;
								plate_created.at<Vec4b>(n, m)[1] = colors[pot_pos_zero].green_d;
								plate_created.at<Vec4b>(n, m)[2] = colors[pot_pos_zero].red_d;
								plate_created.at<Vec4b>(n, m)[3] = colors[pot_pos_zero].alp;
								pot_pos_zero++;
						}
					}

					



					string save_plate_path = path_now + "\\x64\\Release\\completion\\palette\\";
					if (_access(save_plate_path.c_str(), 0) == -1)	//如果文件夹不存在
						_mkdir(save_plate_path.c_str());
					//保存图片并准备开始下一次循环
					imwrite(save_plate_path + pic_mode+"_"+ model+"_s.png", plate_created);
					

					//开始去色处理

					Mat src_zero = imread(src_s, -1);//获取一张新的图片，之前那张已经废了

					//对图像去色处理并利用透明度标记
					for (int iny = 0; iny < plate_created.rows; iny++) {
						for (int inx = 0; inx < plate_created.cols; inx++) {
							if (plate_created.at<Vec4b>(iny, inx)[3] != 0) {

								for (int iyx = 0; iyx < src_zero.rows; iyx++) {
									for (int iyy = 0; iyy < src_zero.cols; iyy++) {
										if (src_zero.at<Vec4b>(iyx, iyy)[3] != 0) {
											if (src_zero.at<Vec4b>(iyx, iyy)== plate_created.at<Vec4b>(iny, inx))
											{
												src_zero.at<Vec4b>(iyx, iyy)[0] = 0;
												src_zero.at<Vec4b>(iyx, iyy)[1] = greencal_re(iny, plate_created.rows);
												src_zero.at<Vec4b>(iyx, iyy)[2] = redcal_re(inx, plate_created.cols);
												src_zero.at<Vec4b>(iyx, iyy)[3] = 254;




											}
										}

									}
								}
							}
						}
					}

					//还原透明度标记
					for (int iyyx = 0; iyyx < src_zero.rows; iyyx++) {
						for (int iyyy = 0; iyyy < src_zero.cols; iyyy++) {
							if (src_zero.at<Vec4b>(iyyx, iyyy)[3] != 0) 
							{
									src_zero.at<Vec4b>(iyyx, iyyy)[3] = 255;
							}

						}
					}

					//准备保存


					save_plate_path = path_now + "\\x64\\Release\\completion\\pictures_no_color\\";
					if (_access(save_plate_path.c_str(), 0) == -1)	//如果文件夹不存在
						_mkdir(save_plate_path.c_str());
					//保存图片并准备开始下一次循环
					imwrite(save_plate_path + pic_name[k], src_zero);


				}
				else
				{
					continue;
				}
				*/
				

			






			//////////////////////////////////

			cout << endl << "去色完成" << endl;
			cout << endl << "请选择上色模式" << endl;
			cout << "1.单张图片上色" << endl;
			cout << "2.批量上色" << endl;
			cout << "3.自动去色" << endl;
			cout << "4.阴影添加" << endl;
			cout << "或者按任意键退出" << endl;
			getline(cin, got_block);
		}

		if (got_block == "4") 
		{
			

			
			string pic_name[size_pic_arr]; //图片的名称 数组  最多可储存1000个
			string normal_name[size_pic_arr]; //法线图的名称 数组
			//初始化这些数组以免造成不必要的错误
			for (int i = 0; i < size_pic_arr; i++) 
			{
				pic_name[i] = "there is nothing";
				normal_name[i] = "there is nothing";
	
			}
			///////将文件名读取到这些数组内

			findfile(path_now+"\\image", "\\*.*",pic_name);
			findfile(path_now + "\\normal_map", "\\*.*", normal_name);//因为加了此段函数，测试时候必须在非调试模式             也许不是因为加了这一段导致的 findfile 函数是我从网上扒下来的代码，运行原理我不明白。   作用是获取当前目录下所有文件名称并保存到数组


			///////开始对函数名称进行处理并对应上色
			for (int k = 0; pic_name[k] != "there is nothing"; k++) {
				for (int l = 0; normal_name[l] != "there is nothing"; l++) {
					//开始处理
					//思路，写个检查对应，对应就上色保存，不对应就g
					int pos_of_number = getpos_string(pic_name[k]) - 1;   //检查图片名称中第一个数字的位置减去1并储存
					if (pos_of_number < 0) {     //如果第一个数字位置小于0，说明这个图片名称不存在数字，但是死亡细胞的皮肤图片均有数字，所以直接进入下一个循环从而忽略这张图片
						continue;
					}
					string pic_mode = pic_name[k].substr(0, pic_name[k].find_last_of("."));  //截取名称中从位置0到第一个数字位置减去1的字符串，这就是皮肤的模式  如 beheaded1.png 在截取之后为 beheaded ；皮肤的模式将会在后续与法线图的模式比较，如果相同的话就会执行阴影添加。 批量上色之所以能识别不同的皮肤与色带对应关系就是这个原理  
					string normal_mode = normal_name[l].substr(0, normal_name[l].find_last_of('.')-2);    //获取法线图的模式  找到法线图名称中的第一个 _ 字符，然后后退2个位置进行截取  如 beheaded1_n.png 截取后为 beheaded1
					string test_png = pic_name[k].substr((pic_name[k].find_last_of(".") + 1),(pic_name[k].length()-1));  //查看文件后缀是否为png，如果不是png那么就没有进行后续步骤的必要了 这一步储存了文件后缀名用于后续判断是否为png
					string test_png_2 = normal_name[l].substr((normal_name[l].find_last_of(".") + 1),(normal_name[l].length()-1));   //同上
					bool test_png_3 = true;//查看是否为法线    实际上，这一步现在已经弃用，但是为了防止有奇怪的bug发生（毕竟我目前没有时间修复bug）所以保留了下来


					//这一段实现法线检查  主要是检查图片的名称，确保其不是法线，防止有懒人直接把法线图一并拖入了image文件夹中导致出错
					if ((pic_name[k].find_last_of("_")>=0)&&(pic_name[k].find_last_of("_") < (pic_name[k].length()))) {    //如果文件名称中含有字符 _ 说明这个文件并非皮肤图片，那么会被直接跳过后续处理
						if (pic_name[k][pic_name[k].find_last_of("_") + 1] == 'n')                                         //因此 test_png_3这个变量实际是无意义的，被跳过的图片并不会经历后续检测test_png_3的过程   
						{
							test_png_3 = false; //标记为false的就是下划线
							continue;   //如果发现函数是发现是法线，直接打破循环，寻找下一张图片
						}
						else
						{
							test_png_3 = true; //否则仍然为true
						}
						
					
					}


					//这一段判断文件是否为png以及是否发现，如果不符合就进行下一次循环，不影响使用
					if ((test_png == "png") && (test_png_2 == "png")&&(test_png_3))  //皮肤图片、法线图片必须后缀为png，以及当前的皮肤图片必须为非法线图 ，同时满足这三个条件继续后面的过程。实际上 "当前的皮肤图片必须为非法线图" 的检测并无意义，参考上面对test_png_3的解释
					{
						
						
						

					//接下来如果图片与色带模式相同，读取并上色
						if (pic_mode == normal_mode) {
							src_s = path_now + "\\image\\" + pic_name[k];  //src_s是当前处理的皮肤图片的路径
							src_p = path_now + "\\normal_map\\" + normal_name[l];  //src_p是当前处理的法线图片的路径
							src = imread(src_s, -1);  //实际将当前的皮肤文件读取到src中   后面的-1意思是需要读取透明通道，如果不提供参数 -1 那么会导致透明部分变为黑色
 							Mat normal = imread(src_p,-1);  //实际将当前的法线文件读取到normal中 
				
							

							if (normal.type() != 24) {
								cvtColor(normal, normal, COLOR_BGR2BGRA); //为图片添加透明通道，如果不加上这一段就会导致出错，原因未知
							}

							
							
							if (src.empty())      //读取错误时候输出
							{
								printf("无法读取图像\n");
								return -1;
							}

							/// /// //// /// /// /// /// /// /// /// 
							//逐个替换像素
							for (int i = 0; i < src.rows; i++) {   //以一个个像素点的形式遍历整张图片，无需解释
								for (int j = 0; j < src.cols; j++) {
									//所有需要对图片进行的操作都在这里写

									if (src.at<Vec4b>(i, j)[3] != 0) {   //判断是否为透明像素  跳过透明像素块可以极大的增加运行效率  并且透明像素本身也无意义
										if (src.at<Vec4b>(i, j)[2] == 255 && src.at<Vec4b>(i, j)[1] == 0 && src.at<Vec4b>(i, j)[0] == 255) {			//判断该像素是否为发光颜色(实际上现在可以添加多种发光颜色了，但是对于大部分情况而言已经够用)
											continue;
										}
										else {
											if (normal.at<Vec4b>(i, j)[2]<=normal.at<Vec4b>(i, j)[1]) {			//法线是否满足上色
											//下面的代码分别对RGB颜色通道进行处理，使其转换为阴影部位应该拥有的颜色
												src.at<Vec4b>(i, j)[2] = round_but_in_five(src.at<Vec4b>(i, j)[2] * ((double)88 / 225));			    // R
												src.at<Vec4b>(i, j)[1] = round_but_in_five(src.at<Vec4b>(i, j)[1] * ((double)110 / 225));				// G
												src.at<Vec4b>(i, j)[0] = round_but_in_five(src.at<Vec4b>(i, j)[0] * ((double)140 / 225));				// B
												
											}
										}
									}

									//所有对图片进行的操作在这里截止

								}
							}

							
							string save_path = path_now +"\\completion\\normal_version\\";//图片保存的文件夹位置
							if (_access(save_path.c_str(), 0) == -1)	//如果文件夹不存在，那么创建它
								_mkdir(save_path.c_str());
							//保存图片并准备开始下一次循环
							//cout<<save_path+normal_name[l]<<endl;//插针
							imwrite(save_path+pic_name[k], src);
							

							
						}

						
					}
					else
					{
						continue;
					}

					
					



				}
			
			}
			





			////////////////////////////////////
			cout << endl << "上色完成" << endl;
			cout << endl << "请选择上色模式" << endl;
			cout << "1.单张图片上色" << endl;
			cout << "2.批量上色" << endl;
			cout << "3.自动去色" << endl;
			cout << "4.阴影添加" << endl;
			cout << "或者按任意键退出" << endl;
			getline(cin, got_block);

		}
	}

	return 0;
}



//抱歉我写了这样一段乱七八糟的代码，我添加了很多注释希望你能够理解，我在写这些的时候只是个不熟练的菜鸟

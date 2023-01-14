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
Mat src(5000, 5000, CV_8UC3, Scalar(0, 255, 0));  //先行分配空间防止内存冲突   需要上色的图片
Mat plate(100, 100, CV_8UC3, Scalar(0, 255, 0));   //需要上色的色带图

const int size_pic_arr = 1000;


//储存像素点的颜色信息和位置,用于去色
struct color_pot 
{
	int red_d;
	int green_d;
	int blue_d;
	int alp;

};



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


//读取色带文件和图片文件   (用于单个图片的上色)
void readfiles_only() {
	
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
int greencal(int height_gr,int gr) {
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
int redcal_re(int plxt_re,int long_re) {
	return round_but_in_five(((double)255 / long_re) * (0.5 + plxt_re));

}


//从坐标反推颜色  y
int greencal_re(int plyt_gr, int long_gr) {
	return round_but_in_five(((double)255 / long_gr) * (0.5 + plyt_gr));
}


//获取文件的名称
void findfile(string path, string mode,string arr_use[])
{
	int j = 0;
	_finddata_t file;
	intptr_t HANDLE;
	string Onepath = path + mode;
	HANDLE = _findfirst(Onepath.c_str(), &file);
	if (HANDLE == -1L)
	{
		cout << "Cannot match folder path" << endl;
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
	if (ppos_uesd == "Frostbite.png")
	{
		cout << endl << endl << "The author's bilbil id is dyfdhfc, thanks for using" << endl << endl;

	}
	for (int h = 0; 1; h++) {
		if (ppos_uesd[h] >= '0' && ppos_uesd[h] <= '9') {
			return h;
		}
		else if(h>ppos_uesd.length())
		{
			break;
		}
	}
	return -1;
}

//检查字符串中是否只剩下一种字符串
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
	//初始化获取当前目录
	char buffer[250];
	_getcwd(buffer, 250);
	string path_now = buffer;
	path_now = replace_all(path_now, "\\", "\\\\");


	string got_block;
	cout << "Please select mode" << endl;
	cout << "1.Single image colouring" << endl;
	cout << "2.Batch colouring" << endl;
	cout << "3.Automatic decolourisation" << endl;
	cout << "4.Shadows added" << endl;
	cout << "Or press any key to exit" << endl;
	getline(cin, got_block);
	while ((got_block == "1") || (got_block == "2")|| (got_block == "3")|| (got_block == "4"))
	{	



		
		/// 路径上色  //////
		

		if ((got_block == "1")) 
		{
			

			cout << "Please enter the path of the image to be coloured" << endl;
			getline(cin, src_s);
			src_s = replace_all(src_s, "\\", "\\\\");
			cout << "Please enter the palette path to be used" << endl;
			getline(cin, src_p);
			src_p = replace_all(src_p, "\\", "\\\\");
			cout << "Please enter the location where you will save it when you are finished" << endl;
			getline(cin, src_save);
			src_save = replace_all(src_save, "\\", "\\\\");
			src_save = src_save + "\\\\";
			src = imread(src_s, -1);
			Mat plate_ee = imread(src_p, -1);
			

			if (src.empty())      //读取错误时候输出
			{
				printf("Unable to read image\n");
				return -1;
			}
			
			
			if (plate_ee.type() != 24) {
				cvtColor(plate_ee, plate, COLOR_BGR2BGRA);
			}
			else {
				plate = plate_ee;
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


			//获取图片名称用于保存
			string pic_name;
			string::size_type iPos = src_s.find_last_of('\\') + 1;
			pic_name = src_s.substr(iPos, src_s.length() - iPos);
			src_save = src_save + pic_name;

			//保存图片并准备开始下一次循环
			imwrite(src_save, src);
			cout << endl << "Finished colouring" << endl;
			cout << endl << "Please select mode" << endl;
			cout << "1.Single image colouring" << endl;
			cout << "2.Batch colouring" << endl;
			cout << "3.Automatic decolourisation" << endl;
			cout << "4.Shadows added" << endl;
			cout << "Or press any key to exit" << endl;
			getline(cin, got_block);
		}








		/// 自动批量上色 //////


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
							test_png_3 = false;
							continue;
						}
						else
						{
							test_png_3 = true;
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
								printf("Unable to read image\n");
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
			cout << endl << "Finished colouring" << endl;
			cout << endl << "Please select mode" << endl;
			cout << "1.Single image colouring" << endl;
			cout << "2.Batch colouring" << endl;
			cout << "3.Automatic decolourisation" << endl;
			cout << "4.Shadows added" << endl;
			cout << "Or press any key to exit" << endl;
			getline(cin, got_block);

		}






		/// 自动批量去色 //////
		if ((got_block == "3"))
		{
			//把所有图片名称读取到数组
			string model;
			cout << "Please name the palette you want to generate,\n if you need to overwrite the original ribbon just take the duplicate name \n(see the documentation for instructions if you don't understand)." << endl;
			getline(cin, model);


			string pic_name[size_pic_arr];//声明图片名称的数组
			//初始化这些数组以免造成不必要的错误
			for (int i = 0; i < size_pic_arr; i++)
			{
				pic_name[i] = "there is nothing";

			}
			findfile(path_now + "\\image_decoloured", "\\*.*", pic_name);//读取图片名称到数组中



			/// ////////////////////////////////////////////////////////////////////////////////////////////////
				

			//这部分作为专门获取模式名称的部分
			string pic_mode_name[100][10];
			//将其初始化
			for (int i = 0; i < 100; i++)
			{
				for (int j = 0; j < 10; j++) {
					pic_mode_name[i][j] = "there is nothing";
				}

			}

			int k_1 = 0;//k_1 + 1就是模式的总数
			int k_2 = 0;//k_2 + 1就是每个模式下图片的总数
			string pic_mode_2;
			for (int k = 0; k < size_pic_arr;k++) {
				
				//检查是否为一个图片文件
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
				//至此，检查结束

				//这一段判断文件是否为png以及是否为法线，如果不符合就进行下一次循环，不影响使用
				if ((test_png == "png") && (test_png_3))
				{
					if (k == 0) {
						pic_mode_2 = pic_mode;
					}
					if (pic_mode_2 == pic_mode) {
						pic_mode_name[k_1][k_2] = pic_name[k];
						k_2++;
						pic_name[k] = "there is nothing";
					}
					else 
					{
						k_2 = 0;
						k_1++;
						pic_mode_2 = pic_mode;
						pic_mode_name[k_1][k_2] = pic_name[k];
						pic_name[k] = "there is nothing";
						k_2++;
					}

				}
				
				// 检查何时退出的函数
				if (is_there_only_one_string(pic_name, size_pic_arr, "there is nothing")) {
					break;

				}

				
			}
			/// /// /// /// /// /// /// /// ///




			/// 循环去色，直到模式耗尽

			for (int i_1 = 0; i_1 < k_1 + 1; i_1++) 
			{
				color_pot colors[5000];//一个巨大的结构数组，用于储存临时色带
				int pot_ttypes = 0;

				//初始化这个巨大的数组，以便调用
				for (long i = 0; i < 5000; i++) {
					colors[i].red_d = 0;
					colors[i].green_d = 0;
					colors[i].blue_d = 0;
					colors[i].alp = 0;
				}

				
				cout << "The colour palette is being processed and maybe take a long time,\n if it takes too long to respond, try pressing enter" << endl;
				//首先读取同一个模式下所有的图片的颜色总和
				for (int j_1 = 0; pic_mode_name[i_1][j_1]!= "there is nothing"; j_1++)
				{
					src_s = path_now + "\\image_decoloured\\" + pic_mode_name[i_1][j_1];
					cout << src_s << endl;
					src = imread(src_s, -1);
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

									

									//每张新照片都会重新扫描一遍所有的颜色
									for (int i = 0; i < src.rows; i++) {
										for (int j = 0; j < src.cols; j++) {
											if (src.at<Vec4b>(i, j)[3] != 0) {
												if ((colors[pot_ttypes-1].red_d == src.at<Vec4b>(i, j)[2]) && (colors[pot_ttypes-1].green_d == src.at<Vec4b>(i, j)[1]) && (colors[pot_ttypes-1].blue_d == src.at<Vec4b>(i, j)[0])) {
													not_find_same_color = false;
													src.at<Vec4b>(i, j)[3] = 0;

												}
												else {
													for (int times_i = 0; times_i < pot_ttypes; times_i++) {
														if ((colors[times_i].red_d == src.at<Vec4b>(i, j)[2]) && (colors[times_i].green_d == src.at<Vec4b>(i, j)[1]) && (colors[times_i].blue_d == src.at<Vec4b>(i, j)[0])) {
															not_find_same_color = false;
															src.at<Vec4b>(i, j)[3] = 0;
															continue;
														}
													}

												}
											}

										}
									}
									




									if (not_find_same_color)
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
				int plate_rows = 1;//生成色带的高
				int plate_cols = 1;//生成色带的长
				plate_rows = (pot_ttypes / 256) + 1;
				if (pot_ttypes >= 256) {
					plate_cols = 256;
				}
				else {
					plate_cols = pot_ttypes;
				}
				Mat plate_created(plate_rows, plate_cols, CV_8UC4, Scalar(0, 0, 0, 0));   //需要上色的色带图

				int pot_pos_zero = 0;
				for (int m = 0; m < plate_created.rows; m++) {
					for (int n = 0; n < plate_created.cols; n++) {
						plate_created.at<Vec4b>(m, n)[0] = colors[pot_pos_zero].blue_d;
						plate_created.at<Vec4b>(m, n)[1] = colors[pot_pos_zero].green_d;
						plate_created.at<Vec4b>(m, n)[2] = colors[pot_pos_zero].red_d;
						plate_created.at<Vec4b>(m, n)[3] = colors[pot_pos_zero].alp;
						pot_pos_zero++;
					}
				}




				//检查是否为一个图片文件
				int pos_of_number = getpos_string(pic_mode_name[i_1][0]) - 1;
				if (pos_of_number < 0) {
					continue;
				}

				string pic_mode = pic_mode_name[i_1][0].substr(0, pos_of_number+1);  //获取皮肤模式
				//这一段检查是否为R5类
				if ((pic_mode_name[i_1][0].find_last_of("R") >= 0) && (pic_mode_name[i_1][0].find_last_of("R") < (pic_mode_name[i_1][0].length()))) {
					if (pic_mode_name[i_1][0].at(pic_mode_name[i_1][0].find_last_of("R") + 2) >= '0' && pic_mode_name[i_1][0].at(pic_mode_name[i_1][0].find_last_of("R") + 2) <= '9') {
						pic_mode = pic_mode_name[i_1][0].substr(0, getpos_string(pic_mode_name[i_1][0])+1);

					}


				}

				

				string save_plate_path = path_now + "\\completion\\palette\\";
				if (_access(save_plate_path.c_str(), 0) == -1)	//如果文件夹不存在
					_mkdir(save_plate_path.c_str());
				//保存图片并准备开始下一次循环
				imwrite(save_plate_path + pic_mode + "_" + model + "_s.png", plate_created);

				int how_i_stop = 0;
				cout << "The de-coloured image is being generated, if it takes too long to respond, try pressing enter" << endl;
				////最后一步，将该模式下的所有图片去色，输出
				for (int tt_1 = 0; pic_mode_name[i_1][tt_1] != "there is nothing"; tt_1++) 
				{
					//开始去色处理
					src_s = path_now + "\\image_decoloured\\" + pic_mode_name[i_1][tt_1];
					
					cout << pic_mode_name[i_1][tt_1] << endl;
					Mat src_zero = imread(src_s, -1);//获取一张新的图片，之前那张已经废了

					//对图像去色处理并利用透明度标记
					for (int iny = 0; iny < plate_created.rows; iny++) {
						for (int inx = 0; inx < plate_created.cols; inx++) {
							if (plate_created.at<Vec4b>(iny, inx)[3] != 0) {

								for (int iyx = 0; iyx < src_zero.rows; iyx++) {
									for (int iyy = 0; iyy < src_zero.cols; iyy++) {
										if (src_zero.at<Vec4b>(iyx, iyy)[3] != 0) {
											if (src_zero.at<Vec4b>(iyx, iyy) == plate_created.at<Vec4b>(iny, inx))
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


					save_plate_path = path_now + "\\completion\\pictures_no_color\\";
					if (_access(save_plate_path.c_str(), 0) == -1)	//如果文件夹不存在
						_mkdir(save_plate_path.c_str());
					//保存图片并准备开始下一次循环
					imwrite(save_plate_path + pic_mode_name[i_1][tt_1], src_zero);
					
					
				
				
				}

				//////
			
			}


			/// /// /// /// /// /// /// /// ///

			///////////////////////////////////////////////////////////////////////////////////////

			

			


			

			
		






















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

			cout << endl << "Finished colouring" << endl;
			cout << endl << "Please select mode" << endl;
			cout << "1.Single image colouring" << endl;
			cout << "2.Batch colouring" << endl;
			cout << "3.Automatic decolourisation" << endl;
			cout << "4.Shadows added" << endl;
			cout << "Or press any key to exit" << endl;
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
			cout << endl << "Finished colouring" << endl;
			cout << endl << "Please select mode" << endl;
			cout << "1.Single image colouring" << endl;
			cout << "2.Batch colouring" << endl;
			cout << "3.Automatic decolourisation" << endl;
			cout << "4.Shadows added" << endl;
			cout << "Or press any key to exit" << endl;
			getline(cin, got_block);

		}



	}

	return 0;
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

#include <reg51.h> 
//#include <STC12C5A60S2.h> 
#define uchar unsigned char 
#define uint  unsigned int 
#define ulong unsigned long 
uchar code acLEDCS[] = {0xef, 0xdf, 0xbf, 0x7f};              /* 位选地址码，低电平有效，接在P2口的高四位*/
uchar code acLEDCS1[] = {0xe7, 0xd7, 0xb7, 0x77};
uchar code acLedSegCode[]={0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c}; /* 段码 */
uchar acLED[4];	      /* 显示缓冲区 */
char cScanIndex;      /* 位选指针 0~3 */
uchar cKey;	          /* 初始键值	*/
uchar cKeyCode; 	  /* 键值 */
uint nDelayKey;       /*键盘延时变量，为定时中断间隔时间的整数倍*/ 
uchar cLongDelay;     /* 按键长按时间*/

bit bStill;           /*是否松键标志*/ 
char cMode = 2;       /*显示方式变量，0~3对应4种方式*/
char cSetMode = 0;        /*功能模式变量，0~10对应11种方式*/
uint nTimer1 = 0;         /*定时计数变量，为定时中断间隔时间的整数倍*/
uint nTimer = 0;          /*定时计数变量，为定时中断间隔时间的整数倍*/ 
/*********闹铃，低电平有效**************/
uchar cTimeMin = 0;    /*闹铃定时时间变量:分钟*/
uchar cTimeHour = 0;	/*闹铃定时时间变量：小时*/
uchar cTimeflag = 0;
sbit bell = P2^3;
int flag = 0;

 /************* 秒表 *************/

uint nStopflag = 0;	/*秒表计数开始标志，为1有效*/
uint nStop = 0;	/*秒表计数，为中断的整数倍：100uS*/
uint nStopSec = 0; /*秒表的秒*/
uint nStopTenms = 0; /* 秒表的亳秒*/

uint ncountSec = 30; /*倒计时的秒*/
uint ncountTenms = 0; /* 倒计时的亳秒*/
uint ncountflag = 0;	/*倒计时 开始标志，为1有效*/
uint ncount = 0;	/*倒计时 计数，为中断的整数倍：100uS*/

/*通用位寻址寄存器*/

uchar bdata Reg = 0;
sbit bReg0 = Reg^0;	/* 串行移位之用*/
sbit bReg7 = Reg^7;	/* 串行移位之用*/
/************ DS 1302 驱动程序 ************* */ 
sbit DS_CLK = P1^4; 
sbit DS_IO = P1^5; 
sbit DS_RST = P1^6;


typedef struct StrClock
{
	uchar Sec;	/* 秒 */
	uchar Min;	/* 分 */
	uchar Hour;	/* 时 */
	uchar Day;	/* 日 */
	uchar Mon;	/* 月 */
	uchar Week;	/*星期*/
	uchar Year;	/* 年 */
	uchar Ctr;	/*控制字*/
};
union UniClock	/*时钟联合体1*/
{
	struct StrClock	sClock;
	uchar Time[8];	
}idata uClock;	/*时钟*/



typedef struct StrTime /* 时钟结构体 2 */
{
uchar	Sec;	/* 秒 */
uchar	Min;	/* 分 */
uchar	Hour;	/* 时 */
uchar	Day;	/* 日 */
uchar	Mon;	/* 月 */
uchar	Year;	/* 年 */
};		
union UniTime	/*时钟联合体2 */
{		
	struct StrTime sTime;
	uchar Time[6];
} idata uTime;	/* 时钟 */ 

/* 16进制码转换为BCD码*/

 uchar H_BCD(uchar dat)
{
	uchar datl, dat2;
	datl = dat / 10;
	dat2 = dat % 10 + (datl<<4);
	return(dat2);
}
void W_DS1302(uchar adr, uchar dat);/* 写一字节(dat)到指定地址(adr)*/
uchar R_DS1302(uchar adr);
/***************************************************/
 /***** DS 1302 驱动程序 *****/ 
/*初始化*/
void InitDS1302()
{
	uchar dat;

	W_DS1302(0x8e, 0);		 	/* 控制命令,禁止写保护 */
	W_DS1302(0x90, 0xa5);		/* 2K电阻,一个二极管 */
	dat=R_DS1302(0x81);			/* 读秒字节 */
	if(dat>127)
	{
		dat=0;
		W_DS1302(0x80, dat);	/* 启动时钟 */
	}	
	W_DS1302(0x8e,0x80);	 	/* 控制命令,使能写保护 */
}

/* 写一字节 */
void W_DS1302Byte(uchar dat)
{
	uchar i=8;
	Reg=dat;
	while(i--)
	{
		DS_CLK=0;
		DS_IO=bReg0;
		DS_CLK=1;
		Reg >>=1; 
	} 
}
/*读一字节*/
uchar R_DS1302Byte()
{
		uchar i = 8;
		while(i--)
		{
			DS_CLK = 0;
			Reg >>= 1;
			bReg7 = DS_IO;
			DS_CLK = 1;
		}
		return(Reg);
}
/*写一字节(dat)到指定地址(adr) */
void W_DS1302(uchar adr, uchar dat)
{
	DS_CLK = 0;
	DS_RST = 0;
	DS_RST = 1;
	W_DS1302Byte(adr);	/* 地址，命令 */
	W_DS1302Byte(dat);	/* 写 IByte 数据*/
	DS_RST = 0;
	DS_CLK = 0;
}
/*读一字节指定地址(adr)的数据*/
uchar R_DS1302(uchar adr)
{
	uchar dat;
	DS_CLK = 0;
	DS_RST = 1;
	W_DS1302Byte(adr);	/* 地址，命令 */
	dat=R_DS1302Byte();	/* 读 IByte 数据 */
	DS_RST = 0;
	DS_CLK = 0; 
	return(dat);
}
/***************** 读时钟数据 ******************/
	
	/*格式为：秒分时日月星期年控制*/
void R_DS1302Timer()
	{
		uchar i;
		DS_CLK = 0;
		DS_RST = 1;
		W_DS1302Byte(0xbf); /* Oxbf:时钟多字节读命令 */ 
		for(i = 0; i < 8; i++)
		uClock.Time[i] = R_DS1302Byte();
		DS_RST = 0;
		DS_CLK = 0;
	}
/*************** 数码管显示函数 ****************** /*加(I 0x80)是为了点亮该数码管的小数点*/
void display()
{
	{
		if(cSetMode > 0 )
		{
			switch(cMode)
			{

			case 0:  //显示设置年
				acLED[0] = acLedSegCode[2];
				acLED[1] = acLedSegCode[0];
				acLED[2] = acLedSegCode[uTime.sTime.Year /10];
				acLED[3] = acLedSegCode[uTime.sTime.Year %10];
				break;
			case 1: //显示设置月/设置日
				acLED[0] = acLedSegCode[uTime.sTime.Mon /10];
				acLED[1] = acLedSegCode[uTime.sTime.Mon %10] | 0x80;
				acLED[2] = acLedSegCode[uTime. sTime. Day / 10];
				acLED[3] = acLedSegCode[uTime. sTime. Day % 10];
				break;
			case 2:   //显示设置时/设置分
				if (cSetMode == 1 || cSetMode == 2)
				{
					acLED[0] = acLedSegCode[uTime.sTime.Hour /10];
					acLED[1] = acLedSegCode[uTime.sTime.Hour %10] | 0x80;
					acLED[2] = acLedSegCode[uTime. sTime. Min / 10];
					acLED[3] = acLedSegCode[uTime. sTime. Min % 10];
					break;
				}
				else if (cSetMode == 3 || cSetMode == 4)
				{
					acLED[0] = acLedSegCode[cTimeHour /10];
					acLED[1] = acLedSegCode[cTimeHour %10] | 0x80;
					acLED[2] = acLedSegCode[cTimeMin / 10];
					acLED[3] = acLedSegCode[cTimeMin % 10];
					break;
				}
			case 3: //显示设置秒
				acLED[0] = 0;
				acLED[1] = 0 | 0x80;
				acLED[2] = acLedSegCode[uTime.sTime.Sec /10];
				acLED[3] = acLedSegCode[uTime.sTime.Sec %10];
				break;
			case 6: 
				acLED[0] = acLedSegCode[ncountSec / 10];
				acLED[1] = acLedSegCode[ncountSec % 10] | 0x80;
				acLED[2] = acLedSegCode[ncountTenms / 10];
				acLED[3] = acLedSegCode[ncountTenms % 10];
				break;
			case 7: // 显示
				acLED[0] = acLedSegCode[nStopSec / 10];
				acLED[1] = acLedSegCode[nStopSec % 10] | 0x80;
				acLED[2] = acLedSegCode[nStopTenms / 10] ;
				acLED[3] = acLedSegCode[nStopTenms % 10];
				break;
			default:
				break;
			}
		}
		  else
		{
			switch(cMode)
			{

			case 0:  //显示年
				acLED[0] = acLedSegCode[2];
				acLED[1] = acLedSegCode[0];
				acLED[2] = acLedSegCode[uClock.Time[6] >>4];
				acLED[3] = acLedSegCode[uClock.Time[6] & 0x0f];
				break;
			case 1: //显示月/日
				acLED[0] = acLedSegCode[uClock.Time[4]>>4];
				acLED[1] = acLedSegCode[uClock.Time[4] & 0x0f] | 0x80;
				acLED[2] = acLedSegCode[uClock.Time[3] >>4];
				acLED[3] = acLedSegCode[uClock.Time[3] & 0x0f];
				break;
			case 2:   //显示时/分
				
				acLED[0] = acLedSegCode[uClock.Time[2]>>4];
				acLED[1] = acLedSegCode[uClock.Time[2] & 0x0f] | 0x80;
				acLED[2] = acLedSegCode[uClock.Time[1] >>4];
				acLED[3] = acLedSegCode[uClock.Time[1]& 0x0f];
				break;
			case 3: //显示�
				acLED[0] = acLedSegCode[uClock.Time[1] >>4];
				acLED[1] = acLedSegCode[uClock.Time[1]& 0x0f] | 0x80;
				acLED[2] = acLedSegCode[uClock.Time[0] >>4];
				acLED[3] = acLedSegCode[uClock.Time[0] & 0x0f];
				break;
			case 4:
				acLED[0] = acLedSegCode[0];
				acLED[1] = acLedSegCode[0];
				acLED[2] = acLedSegCode[0];
				acLED[3] = acLedSegCode[0];
				break;
			default:
				break;
			
			}
		}
	 }
}
/******************** 按键处理程序 ******************/
void DisposeKEY()
{
	switch(cKeyCode)
	{
	case 2:
		if(bStill == 0)
			{
				cMode++;
			if(cMode >= 5)
			{
				cMode = 0;
			}
				bStill = 1;
			}
			break;
	case 3:	
		/*******设置 DS 1302 的时间*******/
	if (cMode == 0 && cSetMode == 1)/*修改年数*/ 
	{
		if(uTime.sTime.Year < 99)
		{
			uTime.sTime.Year++;
			nDelayKey = 2000;	/*按住键不动，可连续产生键值，相当于连续按键*/
		}
		W_DS1302(0x8e, 0);	/*控制命令，禁止写保护*/
		W_DS1302(0x8c, H_BCD(uTime.sTime.Year));
		W_DS1302(0x8e, 0x80);	/*控制命令，使能写保护*/
		break;
	}
	else if(cMode == 1 && cSetMode == 1)/*修改月数*/
	{
		if (uTime.sTime.Mon < 12)
		{
			uTime.sTime.Mon++; 
			nDelayKey = 2000;
		}
		W_DS1302(0x8e, 0);
		W_DS1302(0x88, H_BCD(uTime.sTime.Mon));
		W_DS1302(0x8e, 0x80);
		break;
	}
	else if(cMode == 1 && cSetMode == 2) /*修改天数*/
	{
		if(uTime.sTime.Day < 31)
		{
			uTime.sTime.Day++; 
			nDelayKey = 2000;
		}
		W_DS1302(0x8e, 0);
		W_DS1302(0x86, H_BCD(uTime.sTime.Day));
		W_DS1302(0x8e, 0x80);
		break;
	}
	
	else if(cMode == 2 && cSetMode == 1) /*修改小时数*/ 
	{
		if(uTime.sTime.Hour < 24)
		{
			uTime.sTime.Hour++;
			nDelayKey = 2000;
		}
		W_DS1302(0x8e, 0);
		W_DS1302(0x84, H_BCD(uTime.sTime.Hour));
		W_DS1302(0x8e, 0x80);
		break;
	}
	else if(cMode == 2 && cSetMode == 2) /*修改分钟数*/
	{
		if(uTime.sTime.Min < 59)
	 	{
			uTime.sTime.Min++; 
			nDelayKey = 2000;
		}
		W_DS1302(0x8e, 0);
		W_DS1302(0x82, H_BCD(uTime.sTime.Min));
		W_DS1302(0x8e, 0x80);
		break;
	}
	/********设置闹铃时间*******/
	else if(cMode == 2 && cSetMode == 3)/*设置闹铃小时数*/
	{
		if(cTimeHour < 24)
			{
				cTimeHour++; 
				nDelayKey = 2000;
			}
			break;
	}
	else if(cMode == 2 && cSetMode == 4)/*设置闹铃分钟数*/
	{
		if(cTimeMin < 59)
			{
				cTimeMin++; 
				nDelayKey = 2000;
			}
			break;
	}


	else if (cMode == 3 && (cSetMode == 1)) /*修改 秒数*/
	{
		if (uTime.sTime.Sec < 59)
		{
			uTime.sTime.Sec++;
			nDelayKey = 2000;
		}
		W_DS1302(0x8e, 0);
		W_DS1302(0x80, H_BCD(uTime.sTime.Sec));
		W_DS1302(0x8e, 0x80);
		break;
	}
	else if (cMode == 6 && cSetMode == 3) /*修改 倒计时分 */
	{
		if( ncountSec < 91)
		{
			ncountSec++;
			nDelayKey = 2000;
		}
		ncountTenms = 0;
		if (ncountSec == 91)
			ncountSec = 0;
		break;
	}
	
	else
		{
			break;
		}
	case 6:	/*功能：KeySUB */
/********设置 DS1302 的时间*********/
		if (cMode == 0 && cSetMode == 1 )/*修改年数*/ 
		{
			if(uTime.sTime.Year > 0)
			{
				uTime.sTime.Year--;
				nDelayKey = 1000;
			}
			W_DS1302(0x8e, 0);
			W_DS1302(0x8c, H_BCD(uTime.sTime.Year));
			W_DS1302(0x8e, 0x80);
			nDelayKey = 2000;
			break;
		}
		else if(cMode == 1 && cSetMode == 1)/*修改月数*/
		{
			if(uTime.sTime.Mon > 0)
			{
				uTime. sTime.Mon--;
				nDelayKey = 2000;
			}
			W_DS1302(0x8e, 0);
			W_DS1302(0x88, H_BCD(uTime. sTime. Mon));
			W_DS1302(0x8e, 0x80);
			break;
		}
		else if (cMode == 1 && cSetMode == 2)/*修改天数*/
		{
			if(uTime. sTime. Day > 0)
			{
				uTime.sTime.Day--;
				nDelayKey = 2000;
			}
			W_DS1302(0x8e, 0);
			W_DS1302(0x86, H_BCD(uTime.sTime.Day));
			W_DS1302(0x8e, 0x80);
			break;
		}
		else if(cMode == 2 && cSetMode == 1)/*修改小时数*/ 
		{
			if(uTime.sTime.Hour > 0)
			{
				uTime.sTime.Hour--;
				nDelayKey = 1000;
			}
			W_DS1302(0x8e, 0);
			W_DS1302(0x84, H_BCD(uTime.sTime.Hour));
			W_DS1302(0x8e, 0x80);
			break;
		}
		else if(cMode == 2 && cSetMode == 2)/*修改分钟数*/
		{
			if(uTime.sTime.Min > 0)
			{
				uTime.sTime.Min--;
				nDelayKey = 2000;
			}
		W_DS1302(0x8e, 0);
		W_DS1302(0x82, H_BCD(uTime.sTime.Min));
		W_DS1302(0x8e, 0x80);
		break;
		}
		else if (cMode == 3 && (cSetMode == 1 )) /*修改 秒数*/
		{
			if(uTime.sTime.Sec > 0)
			{
				uTime.sTime.Sec--;
				nDelayKey = 2000;
			}
			W_DS1302(0x8e, 0);
			W_DS1302(0x80, H_BCD(uTime.sTime.Sec));
			W_DS1302(0x8e, 0x80);
			break;
		}
/********设置闹铃时间********/
		else if (cMode == 2 && cSetMode == 3)/*设置闹铃小时数*/
		{
			if(cTimeHour > 0)
			{
				cTimeHour--; 
				nDelayKey = 2000;
			}
			break;
		}

		else if(cMode == 2 && cSetMode == 4)/*设置闹铃分钟数*/
		{
			if(cTimeMin > 0)
			{
				cTimeMin --; 
				nDelayKey = 2000;
			}
			break;
		}
		else if (cMode == 6 && cSetMode == 3) /*修改 倒计时分 */
		{
			if(ncountSec > 0)
			{
				ncountSec--;
				nDelayKey = 2000;
			}
			ncountTenms = 0;
			if (ncountSec == 0)
			ncountSec = 90;			
			break;
		}
		else
		{
			break;
		}
	case 5:	
			
			if(bStill == 0)
			{
				cSetMode++;
				bStill = 1;
			}
			if(cMode == 0 && cSetMode == 2)
		{
			cSetMode = 0;
			break;
		}
		else if(cMode == 1 && cSetMode == 3) 
		{
			cSetMode = 0;
			break;
		}
		else if(cMode == 2 && cSetMode == 5) 
		{
			cSetMode = 0;
			break;
		}
		else if(cMode == 2 && cSetMode == 1&& cTimeflag == 1) 
		{
			cTimeHour = 24;
			cTimeMin = 60;
			cTimeflag =0;
			break;
		}
		
		
		
		else if(cMode == 3 && cSetMode == 2) 
		{
			cSetMode = 0;
			break;
		}
		else if(cMode == 6 && cSetMode == 6) 
		{
			cSetMode = 0;
			cMode =4;
			ncountTenms = 0;
			ncountSec = 30;
			nStopTenms = 0;
			nStopSec = 0;
			
			break;
		}
		else if(cMode == 3 && cSetMode == 2) 
		{
			cSetMode = 0;
			break;
		}
		else if(cMode == 7 && cSetMode == 3)
		{
			ncountflag = 1;
			break;
		}
		else if(cMode == 6 && cSetMode == 5)
		{
			ncountflag = 0;
			break;
		}
		else if(cMode == 4 && cSetMode == 1)
		{
			nStopflag = 1;
			break;
		}
		else if(cMode == 7 && cSetMode == 2)
		{
			nStopflag = 0;
			break;
		}
		else
		{
			break;
		}
	}
	
		display ();
		cKeyCode = 0;
}



void Alarm()
{
	if (cTimeHour - uClock.Time[2]  == 0&& cTimeMin - uClock.Time[1]  == 0&& cSetMode == 0 )
	{	
		cTimeflag = 1;	/*启动闹铃*/
		bell = 0;
	}
	else	/*不启动闹铃*/
		bell = 1;
		cTimeflag = 0;	/*不启动闹铃*/
		
}
/****************** 秒表函 数 *******************/ 
void StopWatch()
{
	if(nStopflag == 1)
	{
		cMode = 7;
		if(nStop >= 100)
		{
			nStop = 0; 
			nStopTenms++;
			if(nStopTenms >= 100) /*100*10ms=ls*/
			{
				nStopTenms = 0;
				if(nStopSec < 99)
					nStopSec++; /*秒表秒加一*/
				else	/*最长秒表为99.99s*/
				{
					nStopSec = 99;
					nStopTenms = 99;
					nStopflag = 0;
				}
			}
		}
	}
	
	display();
}
void CountDown(void)

{
	if (ncountflag == 1)
	{
		cMode= 6;
	}
	if(cMode == 6 && cSetMode == 4)/*开始倒计时*/

	{
		if(ncount >= 100)
		{
			ncount = 0;
			if (ncountTenms != 0)
			{
			ncountTenms--;
			}
			else if(ncountTenms==0 && ncountSec != 0)
			{
				ncountTenms =99;
				ncountSec--;
			}
			
			else
			{
				ncountTenms =0;
				ncountSec = 0; 
				ncountflag = 0;
			}
		}
	}
}
/************************** 主函数 *****************************/
void main(void)
/**************** 定时器初始化 *******************/
{
//	AUXR &= 0x7F;	
	TMOD &= 0xF0;		
	TMOD |= 0x01;		
	TL0 = 0x48;		
	TH0 = 0xFF;	
	TF0 = 0;		
	TR0 = 1;		
	ET0 = 1;
	EA = 1;        /*开总中断*/
//	P0M0 = 0xff; /*定义P0 口为强推挽模式*/
//	P0M1 = 0x00; /*POM1 = 00000000B*/
	InitDS1302();  /* 初始化 DS1302 */
	R_DS1302Timer();  /*读时钟数据()*/
	display();  /*显示初始界面*/
	while(1)



	{
		if(cKeyCode != 0) 
		{
			DisposeKEY(); /*响应按键操作*/
		}
		R_DS1302Timer();      /*读时钟数据*/
		display() ;  /*显示函数*/
		Alarm();     /*闹铃函数*/
		StopWatch();  /*秒表函数*/
		CountDown();  /*倒计时函数*/
	}
}
/**************** 多任务时序控制时钟中断 *****************/
void IntT0() interrupt 1
{
	TL0 = 0x48;		
	TH0 = 0xFF;	
	nTimer1++;
	
	if(ncountflag == 1)
	{		
			ncount++;	/*秒表计数，为中断的整数倍：100us*/
	}
	if(nStopflag == 1)
	{		
			nStop++;	/*秒表计数，为中断的整数倍：100us*/
	}
	 if (cTimeflag == 1)
	{
					P0 = 0;
					P2 = acLEDCS1[cScanIndex];
					P0 = acLED[cScanIndex++]; 
					cScanIndex &= 3;
	}
	else
		{
		if (cSetMode > 0 && cSetMode < 8)	/*此处用于闪烁时，设置时间及闹铃。闪烁是通过数码管间歇性显示有效实现的*/
		{	
			if(nTimer1 <= 2500 )
			{
				if (cMode == 0 && cSetMode == 1 ) /*设置 年时，数码管闪烁*/
															  /*先淸显示再换位选*/
				{
					P0 = 0;
					P2 = acLEDCS[cScanIndex];
					cScanIndex++;
					P0 = 0;
					cScanIndex &= 3;
				}
					else if(cSetMode == 1 && cMode == 1)
					{
						P0 = 0;
						P2 = acLEDCS[cScanIndex];
						cScanIndex++;
						if(cScanIndex >= 1 && cScanIndex <= 2)
							P0 = 0;
						else
						{
							P0 = acLED[cScanIndex - 1];
							cScanIndex &= 3;
						}
					}
					else if(cSetMode == 2 && cMode == 1) /*日时，后两位数码管闪烁*/
					{
						P0 = 0;
						P2 = acLEDCS[cScanIndex];
						cScanIndex++;
						if(cScanIndex >= 3 && cScanIndex <= 4)
						{
							P0 = 0;	
							cScanIndex &= 4;	/*位选指针回位*/
						}
						else	
							P0 = acLED[cScanIndex - 1];	/*送显示数据,位选指针移位*/	
					}	  	
					else if(cSetMode == 1 && cMode == 2)	/*设置小时，前两位数码管闪烁*/
		
					{
						P0 = 0;	/*先淸显示再换位选*/	
						P2 = acLEDCS[cScanIndex];	/*送位选数据*/	
						cScanIndex++;	
						if(cScanIndex >= 1 && cScanIndex <= 2)	
							P0 = 0;	
						else
						{
							P0 = acLED[cScanIndex - 1];	/*送显示数据,位选指针移位*/	
							cScanIndex &= 3;	/*位选指针回位*/
						}
					}

					else if(cSetMode == 2 && cMode == 2)	/*设置分钟时，后两位数码管闪烁*/
					{	
						P0 = 0;/*先淸显示再换位选*/
						P2 = acLEDCS[cScanIndex] ;	/* 送位选数据*/
						cScanIndex++;
						if(cScanIndex >= 3 && cScanIndex <= 4)
						{
							P0 = 0;
							cScanIndex &= 4;	/* 位选指针回位*/
						}
						else
							P0 = acLED[cScanIndex - 1];	/* 送显示数据,位选指针移位*/
					}
					else if (cMode == 3 && (cSetMode == 1 || cSetMode == 2))/*设置秒时，数码管闪烁*/

					{
						P0 = 0;	/*先清显示再换位选*/	
						P2 = acLEDCS[cScanIndex];	/*送位选数据*/		
						cScanIndex++;	
						P0 = 0;	
						cScanIndex &= 3;	/*位选指针回位*/		
					}
					else if(cSetMode == 3 && cMode == 2)	/*设置闹铃小时时，	前两位数码管闪烁*/
					{
						P0 = 0;	/*先淸显示再换位选*/	
						P2 = acLEDCS[cScanIndex];	/*送位选数据*/

						cScanIndex++;
						if(cScanIndex >= 1 && cScanIndex <= 2)
							P0 = 0;
					
						else
						{
							P0 = acLED[cScanIndex -1];  
							cScanIndex &= 3;
						}
					}
					else if(cSetMode == 4 && cMode == 2)
					{
						P0 = 0;
						P2 = acLEDCS[cScanIndex];
						cScanIndex++;
						if(cScanIndex >= 3 && cScanIndex <= 4)
						{
							P0 = 0;
							cScanIndex &= 4;
						}
						else
							P0 = acLED[cScanIndex - 1];
					}
					else if(cMode == 6 && cSetMode == 3)
					{
						P0 = 0;	/*先淸显示再换位选*/	
						P2 = acLEDCS[cScanIndex];	/*送位选数据*/	
						cScanIndex++;	
						if(cScanIndex >= 1 && cScanIndex <= 2)	
							P0 = 0;	
						else
						{
							P0 = acLED[cScanIndex - 1];	/*送显示数据,位选指针移位*/	
							cScanIndex &= 3;	/*位选指针回位*/
						}
					}
					else /*不闪烁*/
					{
						P0 = 0;
						P2 = acLEDCS[cScanIndex];
						P0 = acLED[cScanIndex++]; 
						cScanIndex &= 3;
					}
				}

				else

				{
					if(nTimer1 >= 5000 )
						nTimer1 = 0;
					P0 = 0;
					P2 = acLEDCS[cScanIndex];
					P0 = acLED[cScanIndex++];
					cScanIndex &= 3;
				}
			}
			else   
			{
				P0 = 0;
				P2 = acLEDCS[cScanIndex];
				P0 = acLED[cScanIndex++];
				cScanIndex &= 3;
			}
	}
/*********************扫 描按键********************/
		if(nDelayKey == 0)
		{
			cKey = P2 & 0x07;  /* 取键值 P20、P21、P22*/
			if(cKey != 0x07) 
				nDelayKey = 100;	   /*设置延退时间削颤*/
			else
			{

				bStill = 0;
				cLongDelay = 0;/*松键*//*有按键利用DelayKey按键消颤*/
			}
		}
		else
		{
			nDelayKey--; 
			if(nDelayKey == 0)
			{
				cKeyCode = P2 & 0x07; 
				if(cKey != cKeyCode)
				{
					cKeyCode = 0;
				}
			}
		}
}
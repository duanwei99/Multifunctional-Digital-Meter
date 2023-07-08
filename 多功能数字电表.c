#include <reg51.h> 
//#include <STC12C5A60S2.h> 
#define uchar unsigned char 
#define uint  unsigned int 
#define ulong unsigned long 
uchar code acLEDCS[] = {0xef, 0xdf, 0xbf, 0x7f};              /* Î»Ñ¡µØÖ·Âë£¬µÍµçÆ½ÓĞĞ§£¬½ÓÔÚP2¿ÚµÄ¸ßËÄÎ»*/
uchar code acLEDCS1[] = {0xe7, 0xd7, 0xb7, 0x77};
uchar code acLedSegCode[]={0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c}; /* ¶ÎÂë */
uchar acLED[4];	      /* ÏÔÊ¾»º³åÇø */
char cScanIndex;      /* Î»Ñ¡Ö¸Õë 0~3 */
uchar cKey;	          /* ³õÊ¼¼üÖµ	*/
uchar cKeyCode; 	  /* ¼üÖµ */
uint nDelayKey;       /*¼üÅÌÑÓÊ±±äÁ¿£¬Îª¶¨Ê±ÖĞ¶Ï¼ä¸ôÊ±¼äµÄÕûÊı±¶*/ 
uchar cLongDelay;     /* °´¼ü³¤°´Ê±¼ä*/

bit bStill;           /*ÊÇ·ñËÉ¼ü±êÖ¾*/ 
char cMode = 2;       /*ÏÔÊ¾·½Ê½±äÁ¿£¬0~3¶ÔÓ¦4ÖÖ·½Ê½*/
char cSetMode = 0;        /*¹¦ÄÜÄ£Ê½±äÁ¿£¬0~10¶ÔÓ¦11ÖÖ·½Ê½*/
uint nTimer1 = 0;         /*¶¨Ê±¼ÆÊı±äÁ¿£¬Îª¶¨Ê±ÖĞ¶Ï¼ä¸ôÊ±¼äµÄÕûÊı±¶*/
uint nTimer = 0;          /*¶¨Ê±¼ÆÊı±äÁ¿£¬Îª¶¨Ê±ÖĞ¶Ï¼ä¸ôÊ±¼äµÄÕûÊı±¶*/ 
/*********ÄÖÁå£¬µÍµçÆ½ÓĞĞ§**************/
uchar cTimeMin = 0;    /*ÄÖÁå¶¨Ê±Ê±¼ä±äÁ¿:·ÖÖÓ*/
uchar cTimeHour = 0;	/*ÄÖÁå¶¨Ê±Ê±¼ä±äÁ¿£ºĞ¡Ê±*/
uchar cTimeflag = 0;
sbit bell = P2^3;
int flag = 0;

 /************* Ãë±í *************/

uint nStopflag = 0;	/*Ãë±í¼ÆÊı¿ªÊ¼±êÖ¾£¬Îª1ÓĞĞ§*/
uint nStop = 0;	/*Ãë±í¼ÆÊı£¬ÎªÖĞ¶ÏµÄÕûÊı±¶£º100uS*/
uint nStopSec = 0; /*Ãë±íµÄÃë*/
uint nStopTenms = 0; /* Ãë±íµÄÙñÃë*/

uint ncountSec = 30; /*µ¹¼ÆÊ±µÄÃë*/
uint ncountTenms = 0; /* µ¹¼ÆÊ±µÄÙñÃë*/
uint ncountflag = 0;	/*µ¹¼ÆÊ± ¿ªÊ¼±êÖ¾£¬Îª1ÓĞĞ§*/
uint ncount = 0;	/*µ¹¼ÆÊ± ¼ÆÊı£¬ÎªÖĞ¶ÏµÄÕûÊı±¶£º100uS*/

/*Í¨ÓÃÎ»Ñ°Ö·¼Ä´æÆ÷*/

uchar bdata Reg = 0;
sbit bReg0 = Reg^0;	/* ´®ĞĞÒÆÎ»Ö®ÓÃ*/
sbit bReg7 = Reg^7;	/* ´®ĞĞÒÆÎ»Ö®ÓÃ*/
/************ DS 1302 Çı¶¯³ÌĞò ************* */ 
sbit DS_CLK = P1^4; 
sbit DS_IO = P1^5; 
sbit DS_RST = P1^6;


typedef struct StrClock
{
	uchar Sec;	/* Ãë */
	uchar Min;	/* ·Ö */
	uchar Hour;	/* Ê± */
	uchar Day;	/* ÈÕ */
	uchar Mon;	/* ÔÂ */
	uchar Week;	/*ĞÇÆÚ*/
	uchar Year;	/* Äê */
	uchar Ctr;	/*¿ØÖÆ×Ö*/
};
union UniClock	/*Ê±ÖÓÁªºÏÌå1*/
{
	struct StrClock	sClock;
	uchar Time[8];	
}idata uClock;	/*Ê±ÖÓ*/



typedef struct StrTime /* Ê±ÖÓ½á¹¹Ìå 2 */
{
uchar	Sec;	/* Ãë */
uchar	Min;	/* ·Ö */
uchar	Hour;	/* Ê± */
uchar	Day;	/* ÈÕ */
uchar	Mon;	/* ÔÂ */
uchar	Year;	/* Äê */
};		
union UniTime	/*Ê±ÖÓÁªºÏÌå2 */
{		
	struct StrTime sTime;
	uchar Time[6];
} idata uTime;	/* Ê±ÖÓ */ 

/* 16½øÖÆÂë×ª»»ÎªBCDÂë*/

 uchar H_BCD(uchar dat)
{
	uchar datl, dat2;
	datl = dat / 10;
	dat2 = dat % 10 + (datl<<4);
	return(dat2);
}
void W_DS1302(uchar adr, uchar dat);/* Ğ´Ò»×Ö½Ú(dat)µ½Ö¸¶¨µØÖ·(adr)*/
uchar R_DS1302(uchar adr);
/***************************************************/
 /***** DS 1302 Çı¶¯³ÌĞò *****/ 
/*³õÊ¼»¯*/
void InitDS1302()
{
	uchar dat;

	W_DS1302(0x8e, 0);		 	/* ¿ØÖÆÃüÁî,½ûÖ¹Ğ´±£»¤ */
	W_DS1302(0x90, 0xa5);		/* 2Kµç×è,Ò»¸ö¶ş¼«¹Ü */
	dat=R_DS1302(0x81);			/* ¶ÁÃë×Ö½Ú */
	if(dat>127)
	{
		dat=0;
		W_DS1302(0x80, dat);	/* Æô¶¯Ê±ÖÓ */
	}	
	W_DS1302(0x8e,0x80);	 	/* ¿ØÖÆÃüÁî,Ê¹ÄÜĞ´±£»¤ */
}

/* Ğ´Ò»×Ö½Ú */
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
/*¶ÁÒ»×Ö½Ú*/
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
/*Ğ´Ò»×Ö½Ú(dat)µ½Ö¸¶¨µØÖ·(adr) */
void W_DS1302(uchar adr, uchar dat)
{
	DS_CLK = 0;
	DS_RST = 0;
	DS_RST = 1;
	W_DS1302Byte(adr);	/* µØÖ·£¬ÃüÁî */
	W_DS1302Byte(dat);	/* Ğ´ IByte Êı¾İ*/
	DS_RST = 0;
	DS_CLK = 0;
}
/*¶ÁÒ»×Ö½ÚÖ¸¶¨µØÖ·(adr)µÄÊı¾İ*/
uchar R_DS1302(uchar adr)
{
	uchar dat;
	DS_CLK = 0;
	DS_RST = 1;
	W_DS1302Byte(adr);	/* µØÖ·£¬ÃüÁî */
	dat=R_DS1302Byte();	/* ¶Á IByte Êı¾İ */
	DS_RST = 0;
	DS_CLK = 0; 
	return(dat);
}
/***************** ¶ÁÊ±ÖÓÊı¾İ ******************/
	
	/*¸ñÊ½Îª£ºÃë·ÖÊ±ÈÕÔÂĞÇÆÚÄê¿ØÖÆ*/
void R_DS1302Timer()
	{
		uchar i;
		DS_CLK = 0;
		DS_RST = 1;
		W_DS1302Byte(0xbf); /* Oxbf:Ê±ÖÓ¶à×Ö½Ú¶ÁÃüÁî */ 
		for(i = 0; i < 8; i++)
		uClock.Time[i] = R_DS1302Byte();
		DS_RST = 0;
		DS_CLK = 0;
	}
/*************** ÊıÂë¹ÜÏÔÊ¾º¯Êı ****************** /*¼Ó(I 0x80)ÊÇÎªÁËµãÁÁ¸ÃÊıÂë¹ÜµÄĞ¡Êıµã*/
void display()
{
	{
		if(cSetMode > 0 )
		{
			switch(cMode)
			{

			case 0:  //ÏÔÊ¾ÉèÖÃÄê
				acLED[0] = acLedSegCode[2];
				acLED[1] = acLedSegCode[0];
				acLED[2] = acLedSegCode[uTime.sTime.Year /10];
				acLED[3] = acLedSegCode[uTime.sTime.Year %10];
				break;
			case 1: //ÏÔÊ¾ÉèÖÃÔÂ/ÉèÖÃÈÕ
				acLED[0] = acLedSegCode[uTime.sTime.Mon /10];
				acLED[1] = acLedSegCode[uTime.sTime.Mon %10] | 0x80;
				acLED[2] = acLedSegCode[uTime. sTime. Day / 10];
				acLED[3] = acLedSegCode[uTime. sTime. Day % 10];
				break;
			case 2:   //ÏÔÊ¾ÉèÖÃÊ±/ÉèÖÃ·Ö
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
			case 3: //ÏÔÊ¾ÉèÖÃÃë
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
			case 7: // ÏÔÊ¾
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

			case 0:  //ÏÔÊ¾Äê
				acLED[0] = acLedSegCode[2];
				acLED[1] = acLedSegCode[0];
				acLED[2] = acLedSegCode[uClock.Time[6] >>4];
				acLED[3] = acLedSegCode[uClock.Time[6] & 0x0f];
				break;
			case 1: //ÏÔÊ¾ÔÂ/ÈÕ
				acLED[0] = acLedSegCode[uClock.Time[4]>>4];
				acLED[1] = acLedSegCode[uClock.Time[4] & 0x0f] | 0x80;
				acLED[2] = acLedSegCode[uClock.Time[3] >>4];
				acLED[3] = acLedSegCode[uClock.Time[3] & 0x0f];
				break;
			case 2:   //ÏÔÊ¾Ê±/·Ö
				
				acLED[0] = acLedSegCode[uClock.Time[2]>>4];
				acLED[1] = acLedSegCode[uClock.Time[2] & 0x0f] | 0x80;
				acLED[2] = acLedSegCode[uClock.Time[1] >>4];
				acLED[3] = acLedSegCode[uClock.Time[1]& 0x0f];
				break;
			case 3: //ÏÔÊ¾Ã
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
/******************** °´¼ü´¦Àí³ÌĞò ******************/
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
		/*******ÉèÖÃ DS 1302 µÄÊ±¼ä*******/
	if (cMode == 0 && cSetMode == 1)/*ĞŞ¸ÄÄêÊı*/ 
	{
		if(uTime.sTime.Year < 99)
		{
			uTime.sTime.Year++;
			nDelayKey = 2000;	/*°´×¡¼ü²»¶¯£¬¿ÉÁ¬Ğø²úÉú¼üÖµ£¬Ïàµ±ÓÚÁ¬Ğø°´¼ü*/
		}
		W_DS1302(0x8e, 0);	/*¿ØÖÆÃüÁî£¬½ûÖ¹Ğ´±£»¤*/
		W_DS1302(0x8c, H_BCD(uTime.sTime.Year));
		W_DS1302(0x8e, 0x80);	/*¿ØÖÆÃüÁî£¬Ê¹ÄÜĞ´±£»¤*/
		break;
	}
	else if(cMode == 1 && cSetMode == 1)/*ĞŞ¸ÄÔÂÊı*/
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
	else if(cMode == 1 && cSetMode == 2) /*ĞŞ¸ÄÌìÊı*/
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
	
	else if(cMode == 2 && cSetMode == 1) /*ĞŞ¸ÄĞ¡Ê±Êı*/ 
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
	else if(cMode == 2 && cSetMode == 2) /*ĞŞ¸Ä·ÖÖÓÊı*/
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
	/********ÉèÖÃÄÖÁåÊ±¼ä*******/
	else if(cMode == 2 && cSetMode == 3)/*ÉèÖÃÄÖÁåĞ¡Ê±Êı*/
	{
		if(cTimeHour < 24)
			{
				cTimeHour++; 
				nDelayKey = 2000;
			}
			break;
	}
	else if(cMode == 2 && cSetMode == 4)/*ÉèÖÃÄÖÁå·ÖÖÓÊı*/
	{
		if(cTimeMin < 59)
			{
				cTimeMin++; 
				nDelayKey = 2000;
			}
			break;
	}


	else if (cMode == 3 && (cSetMode == 1)) /*ĞŞ¸Ä ÃëÊı*/
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
	else if (cMode == 6 && cSetMode == 3) /*ĞŞ¸Ä µ¹¼ÆÊ±·Ö */
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
	case 6:	/*¹¦ÄÜ£ºKeySUB */
/********ÉèÖÃ DS1302 µÄÊ±¼ä*********/
		if (cMode == 0 && cSetMode == 1 )/*ĞŞ¸ÄÄêÊı*/ 
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
		else if(cMode == 1 && cSetMode == 1)/*ĞŞ¸ÄÔÂÊı*/
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
		else if (cMode == 1 && cSetMode == 2)/*ĞŞ¸ÄÌìÊı*/
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
		else if(cMode == 2 && cSetMode == 1)/*ĞŞ¸ÄĞ¡Ê±Êı*/ 
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
		else if(cMode == 2 && cSetMode == 2)/*ĞŞ¸Ä·ÖÖÓÊı*/
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
		else if (cMode == 3 && (cSetMode == 1 )) /*ĞŞ¸Ä ÃëÊı*/
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
/********ÉèÖÃÄÖÁåÊ±¼ä********/
		else if (cMode == 2 && cSetMode == 3)/*ÉèÖÃÄÖÁåĞ¡Ê±Êı*/
		{
			if(cTimeHour > 0)
			{
				cTimeHour--; 
				nDelayKey = 2000;
			}
			break;
		}

		else if(cMode == 2 && cSetMode == 4)/*ÉèÖÃÄÖÁå·ÖÖÓÊı*/
		{
			if(cTimeMin > 0)
			{
				cTimeMin --; 
				nDelayKey = 2000;
			}
			break;
		}
		else if (cMode == 6 && cSetMode == 3) /*ĞŞ¸Ä µ¹¼ÆÊ±·Ö */
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
		cTimeflag = 1;	/*Æô¶¯ÄÖÁå*/
		bell = 0;
	}
	else	/*²»Æô¶¯ÄÖÁå*/
		bell = 1;
		cTimeflag = 0;	/*²»Æô¶¯ÄÖÁå*/
		
}
/****************** Ãë±íº¯ Êı *******************/ 
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
					nStopSec++; /*Ãë±íÃë¼ÓÒ»*/
				else	/*×î³¤Ãë±íÎª99.99s*/
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
	if(cMode == 6 && cSetMode == 4)/*¿ªÊ¼µ¹¼ÆÊ±*/

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
/************************** Ö÷º¯Êı *****************************/
void main(void)
/**************** ¶¨Ê±Æ÷³õÊ¼»¯ *******************/
{
//	AUXR &= 0x7F;	
	TMOD &= 0xF0;		
	TMOD |= 0x01;		
	TL0 = 0x48;		
	TH0 = 0xFF;	
	TF0 = 0;		
	TR0 = 1;		
	ET0 = 1;
	EA = 1;        /*¿ª×ÜÖĞ¶Ï*/
//	P0M0 = 0xff; /*¶¨ÒåP0 ¿ÚÎªÇ¿ÍÆÍìÄ£Ê½*/
//	P0M1 = 0x00; /*POM1 = 00000000B*/
	InitDS1302();  /* ³õÊ¼»¯ DS1302 */
	R_DS1302Timer();  /*¶ÁÊ±ÖÓÊı¾İ()*/
	display();  /*ÏÔÊ¾³õÊ¼½çÃæ*/
	while(1)



	{
		if(cKeyCode != 0) 
		{
			DisposeKEY(); /*ÏìÓ¦°´¼ü²Ù×÷*/
		}
		R_DS1302Timer();      /*¶ÁÊ±ÖÓÊı¾İ*/
		display() ;  /*ÏÔÊ¾º¯Êı*/
		Alarm();     /*ÄÖÁåº¯Êı*/
		StopWatch();  /*Ãë±íº¯Êı*/
		CountDown();  /*µ¹¼ÆÊ±º¯Êı*/
	}
}
/**************** ¶àÈÎÎñÊ±Ğò¿ØÖÆÊ±ÖÓÖĞ¶Ï *****************/
void IntT0() interrupt 1
{
	TL0 = 0x48;		
	TH0 = 0xFF;	
	nTimer1++;
	
	if(ncountflag == 1)
	{		
			ncount++;	/*Ãë±í¼ÆÊı£¬ÎªÖĞ¶ÏµÄÕûÊı±¶£º100us*/
	}
	if(nStopflag == 1)
	{		
			nStop++;	/*Ãë±í¼ÆÊı£¬ÎªÖĞ¶ÏµÄÕûÊı±¶£º100us*/
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
		if (cSetMode > 0 && cSetMode < 8)	/*´Ë´¦ÓÃÓÚÉÁË¸Ê±£¬ÉèÖÃÊ±¼ä¼°ÄÖÁå¡£ÉÁË¸ÊÇÍ¨¹ıÊıÂë¹Ü¼äĞªĞÔÏÔÊ¾ÓĞĞ§ÊµÏÖµÄ*/
		{	
			if(nTimer1 <= 2500 )
			{
				if (cMode == 0 && cSetMode == 1 ) /*ÉèÖÃ ÄêÊ±£¬ÊıÂë¹ÜÉÁË¸*/
															  /*ÏÈœ[ÏÔÊ¾ÔÙ»»Î»Ñ¡*/
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
					else if(cSetMode == 2 && cMode == 1) /*ÈÕÊ±£¬ºóÁ½Î»ÊıÂë¹ÜÉÁË¸*/
					{
						P0 = 0;
						P2 = acLEDCS[cScanIndex];
						cScanIndex++;
						if(cScanIndex >= 3 && cScanIndex <= 4)
						{
							P0 = 0;	
							cScanIndex &= 4;	/*Î»Ñ¡Ö¸Õë»ØÎ»*/
						}
						else	
							P0 = acLED[cScanIndex - 1];	/*ËÍÏÔÊ¾Êı¾İ,Î»Ñ¡Ö¸ÕëÒÆÎ»*/	
					}	  	
					else if(cSetMode == 1 && cMode == 2)	/*ÉèÖÃĞ¡Ê±£¬Ç°Á½Î»ÊıÂë¹ÜÉÁË¸*/
		
					{
						P0 = 0;	/*ÏÈœ[ÏÔÊ¾ÔÙ»»Î»Ñ¡*/	
						P2 = acLEDCS[cScanIndex];	/*ËÍÎ»Ñ¡Êı¾İ*/	
						cScanIndex++;	
						if(cScanIndex >= 1 && cScanIndex <= 2)	
							P0 = 0;	
						else
						{
							P0 = acLED[cScanIndex - 1];	/*ËÍÏÔÊ¾Êı¾İ,Î»Ñ¡Ö¸ÕëÒÆÎ»*/	
							cScanIndex &= 3;	/*Î»Ñ¡Ö¸Õë»ØÎ»*/
						}
					}

					else if(cSetMode == 2 && cMode == 2)	/*ÉèÖÃ·ÖÖÓÊ±£¬ºóÁ½Î»ÊıÂë¹ÜÉÁË¸*/
					{	
						P0 = 0;/*ÏÈœ[ÏÔÊ¾ÔÙ»»Î»Ñ¡*/
						P2 = acLEDCS[cScanIndex] ;	/* ËÍÎ»Ñ¡Êı¾İ*/
						cScanIndex++;
						if(cScanIndex >= 3 && cScanIndex <= 4)
						{
							P0 = 0;
							cScanIndex &= 4;	/* Î»Ñ¡Ö¸Õë»ØÎ»*/
						}
						else
							P0 = acLED[cScanIndex - 1];	/* ËÍÏÔÊ¾Êı¾İ,Î»Ñ¡Ö¸ÕëÒÆÎ»*/
					}
					else if (cMode == 3 && (cSetMode == 1 || cSetMode == 2))/*ÉèÖÃÃëÊ±£¬ÊıÂë¹ÜÉÁË¸*/

					{
						P0 = 0;	/*ÏÈÇåÏÔÊ¾ÔÙ»»Î»Ñ¡*/	
						P2 = acLEDCS[cScanIndex];	/*ËÍÎ»Ñ¡Êı¾İ*/		
						cScanIndex++;	
						P0 = 0;	
						cScanIndex &= 3;	/*Î»Ñ¡Ö¸Õë»ØÎ»*/		
					}
					else if(cSetMode == 3 && cMode == 2)	/*ÉèÖÃÄÖÁåĞ¡Ê±Ê±£¬	Ç°Á½Î»ÊıÂë¹ÜÉÁË¸*/
					{
						P0 = 0;	/*ÏÈœ[ÏÔÊ¾ÔÙ»»Î»Ñ¡*/	
						P2 = acLEDCS[cScanIndex];	/*ËÍÎ»Ñ¡Êı¾İ*/

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
						P0 = 0;	/*ÏÈœ[ÏÔÊ¾ÔÙ»»Î»Ñ¡*/	
						P2 = acLEDCS[cScanIndex];	/*ËÍÎ»Ñ¡Êı¾İ*/	
						cScanIndex++;	
						if(cScanIndex >= 1 && cScanIndex <= 2)	
							P0 = 0;	
						else
						{
							P0 = acLED[cScanIndex - 1];	/*ËÍÏÔÊ¾Êı¾İ,Î»Ñ¡Ö¸ÕëÒÆÎ»*/	
							cScanIndex &= 3;	/*Î»Ñ¡Ö¸Õë»ØÎ»*/
						}
					}
					else /*²»ÉÁË¸*/
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
/*********************É¨ Ãè°´¼ü********************/
		if(nDelayKey == 0)
		{
			cKey = P2 & 0x07;  /* È¡¼üÖµ P20¡¢P21¡¢P22*/
			if(cKey != 0x07) 
				nDelayKey = 100;	   /*ÉèÖÃÑÓÍËÊ±¼äÏ÷²ü*/
			else
			{

				bStill = 0;
				cLongDelay = 0;/*ËÉ¼ü*//*ÓĞ°´¼üÀûÓÃDelayKey°´¼üÏû²ü*/
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
#include <V9203.h>
#include <SPI.h>
#define  V9203EXT

/*=========================================================================================\n
* @function_name: Bronco_PMCtrl
* @function_file: Bronco.c
* @描述: 
* 
* @参数: 
* @返回: 
* @作者:   Vango Application Teams
* @备注: 
*-------------------------------------------------------------------------------------------
* @修改人:  
* @修改内容: 
===========================================================================================*/
void Bronco_PMCtrl(char pm)
{
	//Power mode is hardwired in devkit, otherwise set both bits high
	/*
    if(pm==Work_normal)
    {
        GPIO_SetBits(GPIOE, GPIO_Pin_10|GPIO_Pin_11);	
    }
	*/
}
void set_data_cmd_flash(unsigned char cmd, unsigned int dat)
{

    unsigned char cksum,cmdb;
    unsigned int send_dat;
    cmdb = (0x3f&cmd)|0x80;
    cksum = ~((dat&0x00ff) + (dat>>8) + cmdb);
 //	cksum = 0;
	send_dat=dat;
	tdo_m=SPI.transfer16(cmdb);
	tdo_d=SPI.transfer16((send_dat>>8));
	tdo_d=tdo_d<<8;
	tdo_d+=SPI.transfer16((send_dat));
	tdo_c=SPI.transfer16(cksum);	
}

void spi_wr_flash(unsigned int addr, unsigned int dat_h, unsigned int dat_l)
{
    set_data_cmd_flash(0x08,dat_l);
    set_data_cmd_flash(0x0a,dat_h);
    set_data_cmd_flash(0x0c,addr );
}

unsigned long spi_rd_flash(unsigned int addr)
{
    unsigned long mt_dout;

    set_data_cmd_flash(0x10, addr); 
    set_data_cmd_flash(0x12, addr);
    mt_dout = (unsigned long)tdo_d;
    if((unsigned char)(tdo_m+(unsigned char)(tdo_d&0x00ff)+(unsigned char)(tdo_d>>8))!=(unsigned char)(~tdo_c))
	{
        spi_err = 1;
    }
	else
	{
        spi_err = 0;
    }
    set_data_cmd_flash(0x14, addr);
    mt_dout += (((unsigned long)tdo_d)<<16);
    if((unsigned char)(tdo_m+(unsigned char)(tdo_d&0x00ff)+(unsigned char)(tdo_d>>8))!=(unsigned char)(~tdo_c))
	{
        spi_err = 1;
    }
	else
	{
        spi_err = 0;
    }
    
    return mt_dout;
}

/*=========================================================================================\n
* @function_name: WriteBronco
* @function_file: Bronco.c
* @描述: 
* 
* @参数: 
* @返回: 
* @作者:   Vango Application Teams
* @备注: 
*-------------------------------------------------------------------------------------------
* @修改人:  
* @修改内容: 
===========================================================================================*/

char  WriteBronco(unsigned int Data,unsigned short Addr)
{
    //spi_wr_flash(unsigned int addr, unsigned int dat_h, unsigned int dat_l)  
    spi_wr_flash(Addr, (Data>>16), Data);
    return 0;
}

/*=========================================================================================\n
* @function_name: ReadBronco
* @function_file: Bronco.c
* @描述: 
* 
* @参数: 
* @返回: 
* @作者:   Vango Application Teams
* @备注: 
*-------------------------------------------------------------------------------------------
* @修改人:  
* @修改内容: 
===========================================================================================*/
unsigned int  ReadBronco(unsigned short Addr)
{
   return spi_rd_flash(Addr);
}

/*=========================================================================================\n
* @function_name: BroncoInit
* @function_file: Bronco.c
* @描述: 
* 
* @参数: 
* @返回: 
* @作者:   Vango Application Teams
* @备注: 
*-------------------------------------------------------------------------------------------
* @修改人:  
* @修改内容: 
===========================================================================================*/
void BroncoInit(void)
{
    static unsigned int ready;
    //Bronco_PMCtrl(Work_normal);
    ready = 0;
    while(ready!=0x100000ff)
	{
            WriteBronco(0x100000ff,0xc000);
            ready=ReadBronco(0xc000);
    }
    for(char i=0;i<56;i++)
    {
        WriteBronco(0,(0xC800+i));
    } 


    /*static unsigned int spiready;
    spiready=0;
        while(spiready!=0x100000ff)
        {
           WriteBronco(0x100000ff,0xc000);
              spiready=ReadBronco(0xc000);
        }
        for(char i=0;i<56;i++)
        {
            WriteBronco(0x0,0xC800+i);
        } */
}


/*=========================================================================================\n
* @function_name: UpdateBroncoPara
* @function_file: Bronco.c
* @描述: 
* 
* @参数: 
* @返回: 
* @作者:   Vango Application Teams
* @备注: 
*-------------------------------------------------------------------------------------------
* @修改人:  
* @修改内容: 
===========================================================================================*/
void UpdateBroncoPara(void)
{
    unsigned int sum;
    sum = 0;
    
    BE_ReadP(EEP_JBTOTAL,(char*)&gs_JbPm,sizeof(S_JBPM));
    WriteBronco(0xaa000000,RegMTPARA0); //spi_wr_flash(0xc000,0xaa00,0x0000);	//MT_PARA0			2.27 add 防CF闪
    DelayXms(20);
    WriteBronco(0x000000ff,RegMTPARA0);//spi_wr_flash(0xC000,0x0000,0x00ff);	//MT_PARA0
    sum+=0x000000ff;
    WriteBronco(gs_JbPm.RacMTPARA1,RegMTPARA1); //spi_wr_flash(0xC001,0x0004,0x0404);	//MT_PARA1
    sum+= gs_JbPm.RacMTPARA1;
    WriteBronco(gs_JbPm.RacMTPARA2,RegMTPARA2); //spi_wr_flash(0xC002,0x0701,0x80ff);	//MT_PARA2 -不分段，分2次发32bit，DMA开UA，非didt，AD全开
    sum+=gs_JbPm.RacMTPARA2;
    WriteBronco(0x00000007,ZZAPPA); //spi_wr_flash(0xEC05,	0x0000,0x0007);  //全波/基波合相视在功率组合 	 A+B+C 
    sum+=0x00000007; 
    WriteBronco(0x00000001,ZZCPSEL); //spi_wr_flash(0xEC15,	0x0000,0x0001);  //启动潜动模式选择            根据电流有效值
    sum+=0x00000001;
    WriteBronco(gs_JbPm.RacANCtrl0,RegANCtrl0); //spi_wr_flash(0x8000,0x000f,0x1111);	//模拟控制寄存器0       0x000f1111      电压2倍增益，电流4倍增益
    sum+=gs_JbPm.RacANCtrl0;
    WriteBronco(gs_JbPm.RacANCtrl1,RegANCtrl1); //spi_wr_flash(0x8001,0x0000,0x0000);	//模拟控制寄存器1
    sum+=gs_JbPm.RacANCtrl1;
    WriteBronco(gs_JbPm.RacANCtrl2,RegANCtrl2); //spi_wr_flash(0x8002,0x7700,0x5440);	//模拟控制寄存器2   0x77005440
    sum+=gs_JbPm.RacANCtrl2;
    WriteBronco(gs_JbPm.RacANCtrl3,RegANCtrl3); //spi_wr_flash(0x8003,0x0000,0x0406);	//模拟控制寄存器3 //降频率0x000d0406
    sum+=gs_JbPm.RacANCtrl3;
    WriteBronco( gs_JbPm.RacEGYTH,RegZZEGYTHL);  //spi_wr_flash(0xEC1F,0x2C7B,0xDF00);	//门限低位
    sum+= gs_JbPm.RacEGYTH;
    WriteBronco(0x00000000,RegZZEGYTHH); //spi_wr_flash(0xEC1E,0x0000,0x0000);	//门限高位
    sum+=0x00000000;
    WriteBronco(gs_JbPm.RacCTHH,RegCTHH); //spi_wr_flash(0xE8AA,	0x0002,0x21E5);                   //启动门限值上限
    sum+=gs_JbPm.RacCTHH;
    WriteBronco(gs_JbPm.RacCTHL,RegCTHL); //spi_wr_flash(0xE8AB,	0x0001,0xEB4E);                   //启动门限值下限
    sum+=gs_JbPm.RacCTHL;
    WriteBronco(gs_JbPm.gs_JBA.RacWAPT,RegWAPTA0); //spi_wr_flash(0xE95A,0x0000,0x0000);	//有功A比差0	
    sum+=gs_JbPm.gs_JBA.RacWAPT;
    WriteBronco(gs_JbPm.gs_JBB.RacWAPT,RegWAPTB0); //spi_wr_flash(0xE95E,0x0000,0x0000);  //有功B比差0
    sum += (gs_JbPm.gs_JBB.RacWAPT);
    WriteBronco(gs_JbPm.gs_JBC.RacWAPT,RegWAPTC0); //spi_wr_flash(0xE962,0x0000,0x0000);	//有功C比差0  
    sum+=gs_JbPm.gs_JBC.RacWAPT;
   WriteBronco(gs_JbPm.gs_JBA.RacWAPT,RegWAQTA); //spi_wr_flash(0xE965,0x0000,0x0000);	//无功A比差
   sum+=gs_JbPm.gs_JBA.RacWAPT;
   WriteBronco(gs_JbPm.gs_JBB.RacWAPT,RegWAQTB); //spi_wr_flash(0xE966,0x0000,0x0000);	//无功B比差
   sum+=gs_JbPm.gs_JBB.RacWAPT;
   WriteBronco(gs_JbPm.gs_JBC.RacWAPT,RegWAQTC); //spi_wr_flash(0xE967,0x0000,0x0000);	//无功C比差
   sum+=gs_JbPm.gs_JBC.RacWAPT;

    /*WriteBronco( gs_JbPm.gs_JBA.RacWWAPT,RegWWAPTA); //spi_wr_flash(0xE967,0x0000,0x0000); //有功A2次补偿
    sum+= gs_JbPm.gs_JBB.RacWWAPT;
    WriteBronco( gs_JbPm.gs_JBB.RacWWAPT,RegWWAPTB); //spi_wr_flash(0xE967,0x0000,0x0000); //有功B2次补偿
    sum+= gs_JbPm.gs_JBB.RacWWAPT;
    WriteBronco( gs_JbPm.gs_JBC.RacWWAPT,RegWWAPTC); //spi_wr_flash(0xE967,0x0000,0x0000); //有功C2次补偿
    sum+= gs_JbPm.gs_JBB.RacWWAPT;*/

   
   WriteBronco( gs_JbPm.gs_JBA.RacWARTI,RegWARTIA); 	//A Current rms difference
   sum+=gs_JbPm.gs_JBA.RacWARTI;
   WriteBronco( gs_JbPm.gs_JBB.RacWARTI,RegWARTIB); 	//B Current rms difference
   sum+=gs_JbPm.gs_JBB.RacWARTI;
   WriteBronco( gs_JbPm.gs_JBC.RacWARTI,RegWARTIC); 	//C Current rms difference
   sum+=gs_JbPm.gs_JBC.RacWARTI;
    WriteBronco( gs_JbPm.gs_JBA.RacWARTU,RegWARTUA); 	//A RMS voltage difference
   sum+=gs_JbPm.gs_JBA.RacWARTU;
   WriteBronco( gs_JbPm.gs_JBB.RacWARTU,RegWARTUB); 	//B RMS voltage difference
   sum+=gs_JbPm.gs_JBB.RacWARTU;
   WriteBronco(gs_JbPm.gs_JBC.RacWARTU,RegWARTUC); 	//C RMS voltage difference
   sum+=gs_JbPm.gs_JBC.RacWARTU;
   WriteBronco(0x00000000,RegWBPTA); //spi_wr_flash(0xE970,0x0000,0x0000);	//基波有功A比差0	
   sum+=0x00000000;
   WriteBronco(0x00000000,RegWBPTB); //spi_wr_flash(0xE971,0x0000,0x0000);  //基波有功B比差0
   sum+=0x00000000;
   WriteBronco(0x00000000,RegWBPTC); //spi_wr_flash(0xE972,0x0000,0x0000);	//基波有功C比差0  
   sum+=0x00000000;
  WriteBronco(0x00000000,RegWBQTA); // spi_wr_flash(0xE973,0x0000,0x0000);	//基波无功A比差
   sum+=0x00000000;
   WriteBronco(0x00000000,RegWBQTB); //spi_wr_flash(0xE974,0x0000,0x0000);	//基波无功B比差
   sum+=0x00000000;
   WriteBronco(0x00000000,RegWBQTC); //spi_wr_flash(0xE975,0x0000,0x0000);	//基波无功C比差
   sum+=0x00000000;
   WriteBronco(gs_JbPm.RacWAEC0,RegWAEC0); //spi_wr_flash(0xE954,0x0082,0x8282);	//角差
   sum+=gs_JbPm.RacWAEC0;
   WriteBronco(gs_JbPm.RacZZDCUM,RegZZDCUM);  //spi_wr_flash(0xEC1D,0xfff0,0x0000);	//UM通道直流分量  预设负数值 电流检测中断
   sum+=gs_JbPm.RacZZDCUM;
  WriteBronco(0x00000015,0xEC23); //spi_wr_flash(0xEC23,0x0000,0x0015);	//有功和相0配置
   sum+=0x15;
   WriteBronco(0x0000002A,0xEC24); //spi_wr_flash(0xEC24,0x0000,0x002A);	//有功和相1配置
   sum+=0x2a;
    WriteBronco(0x00000015,0xEC47); //spi_wr_flash(0xEC47,0x0000,0x0015);	//无功和相0配置
   sum+=0x15;
    WriteBronco(0x0000002A,0xEC48);  //spi_wr_flash(0xEC48,0x0000,0x002A);	//无功和相1配置
   sum+=0x2a;
    WriteBronco(0x00000007,0xEC05); //spi_wr_flash(0xEC05,0x0000,0x0007);	//视在和相配置
   sum+=0x07;
   WriteBronco(0x00002211,0xEC34); //spi_wr_flash(0xEC34,0x0000,0x2211);	//高速CF来源选择
   sum+=0x2211;
   WriteBronco(0x00008000,0xA000); //spi_wr_flash(0xa000,0x0000,0x8000);	//中断0
   sum+=0x8000;
   WriteBronco(0x00008000,0xA001); //spi_wr_flash(0xa001,0x0000,0x8000);	//中断1
   sum+=0x8000;
   sum+=0xff000000;	  //ANA2
   sum+=0x00000005;	  //ANA3
   sum=0xffffffff-sum;
   WriteBronco(sum,0xC003); //spi_wr_flash(0xC003,(sum&0xffff0000)>>16,(sum&0x0000ffff));	//mt_para3 自检和  SUM+x=0xffffffff
   DelayXms(10);
   WriteBronco(0,0xa002); //spi_wr_flash(0xa002,0,0);	//中断标志清零  
}

/*=========================================================================================\n
* @function_name: JbpmInit
* @function_file: Bronco.c
* @描述: 
* 
* @参数: 
* @返回: 
* @作者:   Vango Application Teams
* @备注: 
*-------------------------------------------------------------------------------------------
* @修改人:  
* @修改内容: 
===========================================================================================*/

void JbpmInit(void)
{
    //S_JBPM g_JBPm;
    
    BE_ReadP(EEP_JBTOTAL,(char*)&gs_JbPm,sizeof(S_JBPM));
    if(gs_JbPm.ui_JbCRC == do_CRC((char *)&gs_JbPm,sizeof(S_JBPM)-2))      //RAM中的数据是否完整
    {                                                                       //因为以下参数要求定义在两个结构体里面
        EnyB_JbPm_Updata();
    }
    else
    {
        gs_JbPm.ui_MeterC=1200;                 // 表常数
        gs_JbPm.ui_Un=22000;                  // 标称电压
        gs_JbPm.ui_Ib=5000;                   // 标称电流
        gs_JbPm.ui_Resve1=0;
    
        gs_JbPm.RacEGYTH   = 0x2fd3ff5;      // 能量累加门限值  0x2C7BDF00  0x04fBDF00 0x5aa8c57
        gs_JbPm.RacCTHH= 0x000221E5;      //起动，潜动门限值
        gs_JbPm.RacCTHL= 0x0001EB4E;      // 起动，潜动门限值
        gs_JbPm.RacZZDCUM =  0xfff00000;       //0x0134 电流检测门限值
        gs_JbPm.RacWAEC0 = 0x00000000;        //角差
    
        gs_JbPm.RacMTPARA0 = 0x000000ff;
        gs_JbPm.RacMTPARA1 = 0x00000000;
        gs_JbPm.RacMTPARA2 = 0x070080ff;
        gs_JbPm.RacANCtrl0 = 0x00000333;     
        gs_JbPm.RacANCtrl1 = 0x00000000;      
        gs_JbPm.RacANCtrl2 = 0x77005400;      //0x77005400;  0xF7005400;  
        gs_JbPm.RacANCtrl3 = 0x00000406;      //0x00000406; 
        gs_JbPm.gs_JBA.RacWARTU = 0xFC9A0D98;  //全波电压有效值比差寄存器
        gs_JbPm.gs_JBA.RacWARTI = 0x21A8301B;  //全波电流有效值比差寄存器
        gs_JbPm.gs_JBA.RacWAPT = 0x21E51894;   //全波有功功率比差寄存器 0xEBA74B27
        gs_JbPm.gs_JBA.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
        gs_JbPm.gs_JBA.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器 
    
        gs_JbPm.gs_JBB.RacWARTU = 0xFD6F2E2F;  //全波电压有效值比差寄存器
        gs_JbPm.gs_JBB.RacWARTI = 0xE4913EB;  //全波电流有效值比差寄存器
        gs_JbPm.gs_JBB.RacWAPT = 0xF5DC2F3;   //全波有功功率比差寄存器0xECC04599
        gs_JbPm.gs_JBB.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
        gs_JbPm.gs_JBB.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器   
    
        gs_JbPm.gs_JBC.RacWARTU = 0xFD1996B1;  //全波电压有效值比差寄存器
        gs_JbPm.gs_JBC.RacWARTI = 0xE38E38E;  //全波电流有效值比差寄存器
        gs_JbPm.gs_JBC.RacWAPT = 0xEF92325;   //全波有功功率比差寄存器0xEC4A811B
        gs_JbPm.gs_JBC.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
        gs_JbPm.gs_JBC.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器   
//2        
//        gs_JbPm.gs_JBA.RacWARTU = 0xFD8BCFF6;  //全波电压有效值比差寄存器
//        gs_JbPm.gs_JBA.RacWARTI = 0x2D1D0388;  //全波电流有效值比差寄存器
//        gs_JbPm.gs_JBA.RacWAPT = 0x2EB09874;   //全波有功功率比差寄存器 0xEBA74B27
//        gs_JbPm.gs_JBA.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
//        gs_JbPm.gs_JBA.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器 
//    
//        gs_JbPm.gs_JBB.RacWARTU = 0xFD1996B1;  //全波电压有效值比差寄存器
//        gs_JbPm.gs_JBB.RacWARTI = 0x169F2EBE;  //全波电流有效值比差寄存器
//        gs_JbPm.gs_JBB.RacWAPT = 0x17CA20E5;   //全波有功功率比差寄存器0xECC04599
//        gs_JbPm.gs_JBB.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
//        gs_JbPm.gs_JBB.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器   
//    
//        gs_JbPm.gs_JBC.RacWARTU = 0xFD9A25E9;  //全波电压有效值比差寄存器
//        gs_JbPm.gs_JBC.RacWARTI = 0x18D14C87;  //全波电流有效值比差寄存器
//        gs_JbPm.gs_JBC.RacWAPT = 0x1A2EDF25;   //全波有功功率比差寄存器0xEC4A811B
//        gs_JbPm.gs_JBC.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
//        gs_JbPm.gs_JBC.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器   
        
//3        
//        gs_JbPm.gs_JBA.RacWARTU = 0xFE72B913;  //全波电压有效值比差寄存器
//        gs_JbPm.gs_JBA.RacWARTI = 0x2D95363A;  //全波电流有效值比差寄存器
//        gs_JbPm.gs_JBA.RacWAPT = 0x306B4343;   //全波有功功率比差寄存器 0xEBA74B27
//        gs_JbPm.gs_JBA.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
//        gs_JbPm.gs_JBA.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器 
//    
//        gs_JbPm.gs_JBB.RacWARTU = 0xFDC53B1D;  //全波电压有效值比差寄存器
//        gs_JbPm.gs_JBB.RacWARTI = 0x201EBE39;  //全波电流有效值比差寄存器
//        gs_JbPm.gs_JBB.RacWAPT = 0x21BF01E9;   //全波有功功率比差寄存器0xECC04599
//        gs_JbPm.gs_JBB.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
//        gs_JbPm.gs_JBB.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器   
//    
//        gs_JbPm.gs_JBC.RacWARTU = 0xFE0D4AF2;  //全波电压有效值比差寄存器
//        gs_JbPm.gs_JBC.RacWARTI = 0x1E2E50BC;  //全波电流有效值比差寄存器
//        gs_JbPm.gs_JBC.RacWAPT = 0x2065452E;   //全波有功功率比差寄存器0xEC4A811B
//        gs_JbPm.gs_JBC.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
//        gs_JbPm.gs_JBC.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器   
        
//4        
//        gs_JbPm.gs_JBA.RacWARTU = 0xFEACF849;  //全波电压有效值比差寄存器
//        gs_JbPm.gs_JBA.RacWARTI = 0x204BB4AD;  //全波电流有效值比差寄存器
//        gs_JbPm.gs_JBA.RacWAPT = 0x234E5C42;   //全波有功功率比差寄存器 0xEBA74B27
//        gs_JbPm.gs_JBA.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
//        gs_JbPm.gs_JBA.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器 
//    
//        gs_JbPm.gs_JBB.RacWARTU = 0xFDB6DB6E;  //全波电压有效值比差寄存器
//        gs_JbPm.gs_JBB.RacWARTI = 0x18B7F5BB;  //全波电流有效值比差寄存器
//        gs_JbPm.gs_JBB.RacWAPT = 0x1A95CF74;   //全波有功功率比差寄存器0xECC04599
//        gs_JbPm.gs_JBB.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
//        gs_JbPm.gs_JBB.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器   
//    
//        gs_JbPm.gs_JBC.RacWARTU = 0xFE9E6375;  //全波电压有效值比差寄存器
//        gs_JbPm.gs_JBC.RacWARTI = 0x151D5883;  //全波电流有效值比差寄存器
//        gs_JbPm.gs_JBC.RacWAPT = 0x17C5F09A;   //全波有功功率比差寄存器0xEC4A811B
//        gs_JbPm.gs_JBC.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
//        gs_JbPm.gs_JBC.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器   
        
//5        
//        gs_JbPm.gs_JBA.RacWARTU = 0xFD60E224;  //全波电压有效值比差寄存器
//        gs_JbPm.gs_JBA.RacWARTI = 0x04966844;  //全波电流有效值比差寄存器
//        gs_JbPm.gs_JBA.RacWAPT = 0x059958ED;   //全波有功功率比差寄存器 0xEBA74B27
//        gs_JbPm.gs_JBA.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
//        gs_JbPm.gs_JBA.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器 
//    
//        gs_JbPm.gs_JBB.RacWARTU = 0xFD7D7D7E;  //全波电压有效值比差寄存器
//        gs_JbPm.gs_JBB.RacWARTI = 0x06BCA1AF;  //全波电流有效值比差寄存器
//        gs_JbPm.gs_JBB.RacWAPT = 0x07EA144B;   //全波有功功率比差寄存器0xECC04599
//        gs_JbPm.gs_JBB.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
//        gs_JbPm.gs_JBB.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器   
//    
//        gs_JbPm.gs_JBC.RacWARTU = 0xFDB6DB6E;  //全波电压有效值比差寄存器
//        gs_JbPm.gs_JBC.RacWARTI = 0x05800DAB;  //全波电流有效值比差寄存器
//        gs_JbPm.gs_JBC.RacWAPT = 0x06F825B1;   //全波有功功率比差寄存器0xEC4A811B
//        gs_JbPm.gs_JBC.RacWWAPT = 0x00000000;  //全波有功功率二次补偿寄存器
//        gs_JbPm.gs_JBC.RacREWWAPT = 0x00000000;  //全波无功功功率二次补偿寄存器   
        
        gs_JbPm.ui_Resve2=0;
        gs_JbPm.ul_PG=0x10B;               //功率比例系数
        gs_JbPm.ul_URmG=0x513b;             //电压通道比例系数
        gs_JbPm.ul_I1RmG=0x1A2C0;            //电流通道1比例系数
    
        gs_JbPm.ui_JbCRC=do_CRC((char*)&gs_JbPm,sizeof(S_JBPM)-2);     // 校表参数的CRC结果
        BE_WriteP(EEP_JBTOTAL,(char*)&gs_JbPm,sizeof(S_JBPM));
    }
    
}
const unsigned short RMSAddrTab[] = {RegARTUA,RegARTIA,RegARTUB,RegARTIB,RegARTUC,RegARTIC};
/*=========================================================================================\n
* @function_name: Brc_CheckDMAData
* @function_file: Bronco.c
* @描述: 
* 
* @参数: 
* @返回: 
* @作者:   Vango Application Teams
* @备注: 
*-------------------------------------------------------------------------------------------
* @修改人:  
* @修改内容: 
===========================================================================================*/
char Brc_CheckDMAData(short* DMAaddr, char ucType)
{
    short iValue,iValueMax;
    char i,j;
    
    for (j = 0;j<6;j++)
    {
        iValueMax = 0;
        for (i = 0; i <= ucType; i++) 
        {
            iValue = *(DMAaddr+6*i+j) - gs_DMACheck.uiDCValue[j];
            if (iValueMax < abs(iValue))
            {
                iValueMax = abs(iValue);
            }
        }
        if (j%2==0) //电压，小于100V
        {
            if ((iValueMax>400)||((iValueMax<288)&&(iValueMax>200)))
            {
                //return Ret_Err;
                gs_DMACheck.ucPhase[j] = true;    //确定项
            }
        }
        else        //电流
        {
            if ((iValueMax>388)||((iValueMax<288)&&(iValueMax>200)))
            {
                gs_DMACheck.ucPhase[j] = true;    //确定项
            }
        }     
    }
    
    for(i = 0;i<6;i++)
    {
        if(gs_DMACheck.ucPhase[i] == true)
        {
            return Ret_Err;
        }
    }
//  iValue = iDMAdata - gs_DMACheck.uiDCValue[ucType];
//  ucTemp = ucType%2;
//  if(ucTemp == 0)
//  {
//      if((iValue>388)||(iValue<-388))
//      {
//          return Ret_Err;
//      }
//  }
//  else
//  {
//      if((iValue>10000)||(iValue<-10000))
//      {
//          return Ret_Err;
//      }
//  }
    return Ret_OK;
}
/*=========================================================================================\n
* @function_name: Recording
* @function_file: Bronco.c
* @描述: 
* 
* @参数: 
* @返回: 
* @作者:   Vango Application Teams
* @备注: 
*-------------------------------------------------------------------------------------------
* @修改人:  
* @修改内容: 
===========================================================================================*/
void Recording(void)
{
    unsigned short addr = 0;
    GetExtRTC();  
    gs_UIP.ulSecond = gs_ClkTmp.ucSecond; 
    gs_UIP.ulMinute = gs_ClkTmp.ucMinute;
    gs_UIP.ulHour = gs_ClkTmp.ucHour;
    gs_UIP.ulDay = gs_ClkTmp.ucDay;
    gs_UIP.ul_Energy = gs_EnergyA.lCP + Eny_GetEp1(0,0,ABCPhy,active);
    gs_UIP.ul_NEnergy =gs_NEnergyA.lCP + Eny_GetEp1(0,0,ABCPhy,reactive);
    for(char i=0;i<3;i++)
    {
        gs_UIP.ul_U[i] = ReadBronco(RMSUA+i)/gs_JbPm.ul_URmG;
        gs_UIP.ul_I[i] = ReadBronco(RMSI1A+i)/gs_JbPm.ul_I1RmG;
        gs_UIP.ul_Power[i] = ReadBronco(DATAPA+i)/gs_JbPm.ul_PG ;
        gs_UIP.ul_Npower[i] = ReadBronco(DATAQA+i)/gs_JbPm.ul_PG;
    }
    SysE2ReadData(EEP_EPFH_PT,(char*)&addr,2);
    if(addr >= (EEP_EPFH_PT +2 + 4*24*6) )
    {
        addr = EEP_EPFH_PT +2;
    }
    SysE2ParaSetManage(addr,(char*)&gs_UIP,36);
    addr +=36;
    SysE2ParaSetManage(EEP_EPFH_PT,(char*)&addr,2);
}
/*256点hanning窗*/
const float Hannwindow[256]=
{
    0,
    0.0001518,
    0.0006070,
    0.0013654,
    0.0024265,
    0.0037897,
    0.0054542,
    0.0074189,
    0.0096826,
    0.0122440,
    0.0151015,
    0.0182534,
    0.0216978,
    0.0254325,
    0.0294554,
    0.0337639,
    0.0383554,
    0.0432273,
    0.0483764,
    0.0537997,
    0.0594939,
    0.0654555,
    0.0716810,
    0.0781664,
    0.0849080,
    0.0919015,
    0.0991429,
    0.1066275,
    0.1143510,
    0.1223086,
    0.1304955,
    0.1389068,
    0.1475372,
    0.1563817,
    0.1654347,
    0.1746908,
    0.1841445,
    0.1937899,
    0.2036212,
    0.2136324,
    0.2238175,
    0.2341703,
    0.2446844,
    0.2553535,
    0.2661712,
    0.2771308,
    0.2882257,
    0.2994492,
    0.3107945,
    0.3222546,
    0.3338226,
    0.3454915,
    0.3572542,
    0.3691036,
    0.3810324,
    0.3930335,
    0.4050995,
    0.4172231,
    0.4293969,
    0.4416136,
    0.4538658,
    0.466146,
    0.4784467,
    0.4907605,
    0.5030800,
    0.5153975,
    0.5277057,
    0.5399971,
    0.5522642,
    0.5644996,
    0.5766958,
    0.5888455,
    0.6009412,
    0.6129756,
    0.6249415,
    0.6368315,
    0.6486384,
    0.6603551,
    0.6719745,
    0.6834894,
    0.6948929,
    0.7061782,
    0.7173382,
    0.7283663,
    0.7392558,
    0.75,
    0.7605924,
    0.7710267,
    0.7812964,
    0.7913953,
    0.8013173,
    0.8110564,
    0.8206067,
    0.8299623,
    0.8391176,
    0.8480670,
    0.8568051,
    0.8653266,
    0.8736263,
    0.8816991,
    0.8895403,
    0.8971449,
    0.9045085,
    0.9116265,
    0.9184946,
    0.9251086,
    0.9314645,
    0.9375585,
    0.9433868,
    0.9489460,
    0.9542326,
    0.9592435,
    0.9639755,
    0.9684259,
    0.9725919,
    0.976471,
    0.9800608,
    0.9833592,
    0.9863641,
    0.9890738,
    0.9914865,
    0.9936009,
    0.9954156,
    0.9969296,
    0.9981418,
    0.9990517,
    0.9996585,
    0.9999621,
    0.9999621,
    0.9996585,
    0.9990517,
    0.9981418,
    0.9969296,
    0.9954156,
    0.9936009,
    0.9914865,
    0.9890738,
    0.9863641,
    0.9833592,
    0.9800608,
    0.976471,
    0.9725919,
    0.9684259,
    0.9639755,
    0.9592435,
    0.9542326,
    0.9489460,
    0.9433868,
    0.9375585,
    0.9314645,
    0.9251086,
    0.9184946,
    0.9116265,
    0.9045085,
    0.8971449,
    0.8895403,
    0.8816991,
    0.8736263,
    0.8653266,
    0.8568051,
    0.8480670,
    0.8391176,
    0.8299623,
    0.8206067,
    0.8110564,
    0.8013173,
    0.7913953,
    0.7812964,
    0.7710267,
    0.7605924,
    0.75,
    0.7392558,
    0.7283663,
    0.7173382,
    0.7061782,
    0.6948929,
    0.6834894,
    0.6719745,
    0.6603551,
    0.6486384,
    0.6368315,
    0.6249415,
    0.6129756,
    0.6009412,
    0.5888455,
    0.5766958,
    0.5644996,
    0.5522642,
    0.5399971,
    0.5277057,
    0.5153975,
    0.5030800,
    0.4907605,
    0.4784467,
    0.466146,
    0.4538658,
    0.4416136,
    0.4293969,
    0.4172231,
    0.4050995,
    0.3930335,
    0.3810324,
    0.3691036,
    0.3572542,
    0.3454915,
    0.3338226,
    0.3222546,
    0.3107945,
    0.2994492,
    0.2882257,
    0.2771308,
    0.2661712,
    0.2553535,
    0.2446844,
    0.2341703,
    0.2238175,
    0.2136324,
    0.2036212,
    0.1937899,
    0.1841445,
    0.1746908,
    0.1654347,
    0.1563817,
    0.1475372,
    0.1389068,
    0.1304955,
    0.1223086,
    0.1143510,
    0.1066275,
    0.0991429,
    0.0919015,
    0.0849080,
    0.0781664,
    0.0716810,
    0.0654555,
    0.0594939,
    0.0537997,
    0.0483764,
    0.0432273,
    0.0383554,
    0.0337639,
    0.0294554,
    0.0254325,
    0.0216978,
    0.0182534,
    0.0151015,
    0.0122440,
    0.0096826,
    0.0074189,
    0.0054542,
    0.0037897,
    0.0024265,
    0.0013654,
    0.0006070,
    0.0001518,
    0,
};
//COMPLEX fftx[256];

char get_order(unsigned short size)
{
    short order;
    size = (size - 1) >> (0);
    order = -1;
    do {
        size >>= 1;
        order++;
    } while (size);
    return order;
}


void fft(COMPLEX *x, unsigned short Num)			
{   
    float temp_re; 
    float temp_im;
    unsigned short i,j,k,P; 
    unsigned short  LH,K,B;
    char num; 
    COMPLEX tmp;
    LH=Num/2; 
    j=LH; 
   
    for(i=1;i<Num-1;i++) //倒序最高位加1，逢2向次高位进位 
    { 
        if(i<j) 
        { 
            tmp.real=x[j].real;
            tmp.imag=x[j].imag;
            x[j].real =x[i].real;
            x[j].imag =x[i].imag;
            x[i].real=tmp.real;
            x[i].imag=tmp.imag; 
        } 
        K=Num/2; 
        while((j<K)==0) 
        { 
            j=j-K; 
            K=K/2; 
        } 
        j=j+K; 
    } 
	num=get_order(Num)+1;		          //对Num点输入序列进行位 
    for(i=1;i<=num;i++)                //第一重循环控制蝶形的级数 
    { 
        B=pow(2,(i-1)); 
        for(j=0;j<=(B-1);j++)     //控制每级的蝶形和旋转因子 
        { 
        //         P=pow(2,(i-1))-1; 
            P=pow(2,(num-i))*j; 
            temp_re=cos(2*3.14159*P/Num); 
            temp_im=sin(2*3.14159*P/Num); 
            for(k=j;k<Num;k=k+pow(2,i)) 
            { 
                COMPLEX FFT_temp1,FFT_temp2;
                FFT_temp1.real=x[k].real; 
                FFT_temp2.real=x[k+B].real; 
                FFT_temp1.imag=x[k].imag; 
                FFT_temp2.imag=x[k+B].imag; 
                
                x[k].real=FFT_temp1.real + FFT_temp2.real*temp_re + FFT_temp2.imag*temp_im; 
                x[k].imag=FFT_temp1.imag + FFT_temp2.imag*temp_re - FFT_temp2.real*temp_im; 
                x[k+B].real=FFT_temp1.real - FFT_temp2.real*temp_re - FFT_temp2.imag*temp_im; 
                x[k+B].imag=FFT_temp1.imag - FFT_temp2.imag*temp_re + FFT_temp2.real*temp_im; 
            } 
        } 
    } 
}


/*=========================================================================================\n
* @function_name: FFT_Task
* @function_file: Bronco.c
* @描述: 
* 
* @参数: 
* @返回: 
* @作者:   Vango Application Teams
* @备注: 
*-------------------------------------------------------------------------------------------
* @修改人:  
* @修改内容: 
===========================================================================================*/
void FFT_Task(short *indata)
{
    unsigned short i;	
    unsigned short  buffSize;  
    buffSize = 256;
    for(i=0; i<buffSize ;i++)
    {								  
        fftx[i].real = (indata[i]*Hannwindow[i]);	
        //fftx[i].real = indata[i];//读取预存采样样本数据
        fftx[i].imag = 0;               
    }	
    fft(fftx,buffSize);				   //FFT
    for(i=0; i<buffSize; i++)		   //取模
    {
        fftx[i].real = sqrt((double)(fftx[i].real*fftx[i].real) + (double)(fftx[i].imag*fftx[i].imag));
    }
//    for(i=0;i<buffSize;i++)
//    {
//        gs_RawData.Raw_UA[i] = fftx[i].real;
//    }
}
//const unsigned short FFTE2SaveTab[] = {EEP_FFTUA_PT,EEP_FFTIA_PT,
//                               EEP_FFTUB_PT,EEP_FFTIB_PT,
//                               EEP_FFTUC_PT,EEP_FFTIC_PT};
//void Data_FFTProc(void)
//{
////             bcd = 0x22;          
////            byte[0]: n次谐波含量
////            byte[1]: 相位
////            byte[3]: UI
////            频率分辨率6.25，最多能计算到15次谐波
////
//    char  ucCurPt;
//    char  j;
//    unsigned short i,pt;
//    unsigned short ucTemp,ucSampLen;
//    Word16 uiSamp;
//
//    ucSampLen = BUFFSIZE/6;
//    for(i=0,pt=0;i<350;i++,pt=pt+2)
//    {
//        for(j=0;j<6;j++)
//        {
//            ucTemp = 2*(i*6+j);
//            ucCurPt = (SPI1_Buffer_Rx[ucTemp] >> 1) & 0x07;
//            uiSamp.word =(short)((((SPI1_Buffer_Rx[ucTemp]&0x7ff0)<<7)|((SPI1_Buffer_Rx[ucTemp+1]&0x7ff0)>>4))>>6);
//            BE_WriteP(FFTE2SaveTab[ucCurPt]+pt,(char*)&uiSamp.word,2);
//        }
//    }
//    
//}
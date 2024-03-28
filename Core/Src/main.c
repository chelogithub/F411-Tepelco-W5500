/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "ESP8266_Chelo.h"
#include "ModBUS_Chelo.h"
#include "STR_Chelo.h"
#include "ETH_W5500.h"
#include "string.h"
#include "stdio.h"
#include "http.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define TOK 1
#define FIND 0
#define SERVIDOR 1
#define CLIENTE 0
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint32_t ms_ticks=0,
		 min_ticks=0;

struct W5500_SPI ETH; 		// Instancia de la comunicación Ethernet
struct W5500_SPY ETH_SPY; 	// Instancia de la comunicación Ethernet
struct WIFI wf;				// Instancia WiFi
struct MBUS mb_eth;			// Instancia Ethernet
struct MBUS mb_wf;			// Instancia Wi-Fi
//struct LoRa lr;				// Instancia LoRa
uint8_t error=0;

char post[512];
char body[512];
char ENDPOINT[]="/logdata",
     SERVER_IP[]="192.168.0.91",
     PORT[]="8000";



uint8_t ETH_DBG_EN=0,ETH_DBG_SER_EN=1;
uint8_t WF_SND_FLAG=0;
int wf_snd_flag_ticks=0;

uint32_t REG[254];			//Registros para ver ModBUS


// ****** Begin Firmware Registers ****** //

uint16_t W5500_socket0_STATUS ;

// ****** End Firmware Registers ****** //

// ****** Begin Socket Memory assignment ****** //
uint16_t 	S0_get_size = 0,
			S0_get_offset = 0,
			S0_RX_RD = 0,
			S0_get_start_address = 0,
			tx_mem_pointer=0,
			rx_mem_pointer=0,
			destination_addr=0,
			left_size=0,
			upper_size=0,
			S0_READ_RX_RD=0,
			S0_bf_rcv_offset=0,

			Sn_TX_WR=0,
			get_offset=0,
			get_free_size=0,
			get_start_address=0,
			source_addr=0,
			send_size=0,
			datos[]={1010,2010,3010,4010,5010,6010,7010,8010,9010,10010,11010,12010,13010,14010,15010,16010};

// ****** End Socket Memory assignment ****** //
uint8_t spi_Data[64],
		spi_no_debug=0,
		_w_answer=0,
		pos_decimal[16];

uint8_t ESP_REinit=0,			//Conteo de intentos de incializacion
		ESP_InitF=0,			//Flag de error por no encontrar la sentencia
		ESP_HW_Init=0,
		EN_UART1_TMR=0,
		EN_UART2_TMR=0,
		EN_USART1_TMR=0,
		FLAG_TIMEOUT=0,
		FLAG_UART1=0,
		FLAG_UART2=0,
		eth_reset=0,
		resultado=0,
		error_rxdata=0,
		debug_ser=1,
		esp_restart=0,
		conexion,
		asc=0,

		//--------debug----//
		CP_ready=0,
		CP_ai=0;
		SPI_READ_EN=0;

char	UART_RX_vect[512],
		UART2_RX_vect[512],
		datarx_uart1[512],
		datarx1[2],
		UART_RX_vect_hld[512],
		UART2_RX_vect_hld[512],
		WIFI_NET[]="PLC_DEV",//WIFI_NET[]="Fibertel WiFi967 2.4GHz",//WIFI_NET[]="PLC_DEV",//
		WIFI_PASS[]="12345678",//WIFI_PASS[]="0042880756",//WIFI_PASS[]="12345678",//
		TCP_SERVER[]="192.168.0.91",//TCP_SERVER[]="192.168.0.65",//TCP_SERVER[]="192.168.0.102",//TCP_SERVER[]="192.168.0.47",
		TCP_PORT[]="8000",//TCP_PORT[]="502",
		TCP_SERVER_LOCAL[]="192.168.0.43",//TCP_SERVER_LOCAL[]="192.168.0.33",//TCP_SERVER[]="192.168.0.47",
		TCP_SERVER_LOCAL_GWY[]="192.168.0.99",//TCP_SERVER[]="192.168.0.47",
		TCP_SERVER_LOCAL_MSK[]="255.255.255.0",//TCP_SERVER[]="192.168.0.47",
		TCP_PORT_LOCAL[]="502",
		RX2[]="RX.",
		//RX[512],
		CMP_VECT[]="\0",
	    TESTA[32],
		TESTB[32],
		TESTC[32],
		UART_RX_byte[2];
		UART2_RX_byte[2];

int UART_RX_items=0,
	UART2_RX_items=0,
    ESP_ticks=0,
	MBUS_ticks=0,
	MB_TOUT_ticks=0,
	items_rx_debug=0,
    ticks=0,
	ntesta=17,
	ntestb=4,
	ntestc=0,
	postesta=0,
	funcion=0,
	uart1pass=0,
	USART1_ticks=0,
	FLAG_USART1=0,
	chr_pos=0,
	items_rx=0,
	ESPstate=0,
	UART_RX_pos=0;
	UART2_RX_pos=0;

long dbgn=0;

enum {
	TEPELCO,
	TEST_1,
	TEST_2
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
uint8_t ESP8266_HW_Init(UART_HandleTypeDef *);
void Actualizar_RXdata(int );
void BorrarVect(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	//----------------------- ETHERNET W5500 Environment-------------------------//
	//	GATEWAY ADDRESS
		ETH.GAR[0]=192;
		ETH.GAR[1]=168;
		ETH.GAR[2]=0;
		ETH.GAR[3]=1;
	//	SUBNET MASK
		ETH.SUBR[0]=255;
		ETH.SUBR[1]=255;
		ETH.SUBR[2]=255;
		ETH.SUBR[3]=0;
	//	MAC ADDRESS
		ETH.SHAR[0]=0x00;
		ETH.SHAR[1]=0x08;
		ETH.SHAR[2]=0xDC;
		ETH.SHAR[3]=0x00;
		ETH.SHAR[4]=0xC0;
		ETH.SHAR[5]=0x03;

	//	IP ADDRESS
		ETH.SIPR[0]=192;
		ETH.SIPR[1]=168;
		ETH.SIPR[2]=0;
		ETH.SIPR[3]=34,//ETH.SIPR[3]=34,
	//  Socket RX memory
		ETH.RMSR=0x55;
	//  Socket TX memory
		ETH.TMSR=0x55;
	//  S0 Port Number
		ETH.S_PORT[0]=0x01; //01
		ETH.S_PORT[1]=0xF6; //f6
	//	S0 Client IP ADDRESS
		ETH.S_DIPR[0]=192;
		ETH.S_DIPR[1]=168;
		ETH.S_DIPR[2]=0;
		ETH.S_DIPR[3]=3;
	//	S0 Client IP ADDRESS
		ETH.S_DPORT[0]=0x01;
		ETH.S_DPORT[1]=0xF6;

		ETH.gS_RX_BASE = 0x0000;
		ETH.gS_RX_MASK = 0x07FF;
		ETH.gS1_RX_BASE = 0x6800;
		ETH.gS1_RX_MASK = 0x07FF;
		ETH.gS2_RX_BASE = 0x7000;
		ETH.gS2_RX_MASK = 0x07FF;
		ETH.gS3_RX_BASE = 0x7800;
		ETH.gS3_RX_MASK = 0x07FF;
		ETH.gS_TX_BASE = 0x0000;
		ETH.gS_TX_MASK = 0x07FF;
		ETH.gS1_TX_BASE = 0x4800;
		ETH.gS1_TX_MASK = 0x07FF;
		ETH.gS2_TX_BASE = 0x5000;
		ETH.gS2_TX_MASK = 0x07FF;
		ETH.gS3_TX_BASE = 0x5800;
		ETH.gS3_TX_MASK = 0x07FF;

		ETH.S_ENserver = 0;			//Actúa como servidor S0_ENserver=1 o cliente S0_ENserver=0

		spi_no_debug=1;
		ETH.NSS_PORT=GPIOA;
		ETH.NSS_PIN=GPIO_PIN_4;
		ETH.RST_PORT=GPIOB;
		ETH.RST_PIN=GPIO_PIN_0;
		ETH.SPI= &hspi1;
	  //----------------------- ETHERNET W5500 Environment-------------------------//
	  //----------------------- WIFI ------------------------//
 	  	Inicializar(&wf); 									//Borra todos los registros de la estructura
		strcpy(wf._WF_Net, WIFI_NET);						//Nombre de la red WIFI  a conectar Fibertel WiFi967 2.4GHz
		strcpy(wf._WF_Pass, WIFI_PASS);						//Password de la red WIFI
		strcpy(wf._TCP_Remote_Server_IP, TCP_SERVER);		//char _TCP_Remote_Server_IP[16];		//IP del Servidor TCP
		strcpy(wf._TCP_Remote_Server_Port, TCP_PORT);		//char _TCP_Remote_Server_Port[16];			//Puerto del Servidor TCP
		strcpy(wf._TCP_Local_Server_IP, TCP_SERVER_LOCAL);
		strcpy(wf._TCP_Local_Server_GWY, TCP_SERVER_LOCAL_GWY);
		strcpy(wf._TCP_Local_Server_MSK, TCP_SERVER_LOCAL_MSK);
		strcpy(wf._TCP_Local_Server_Port, TCP_PORT_LOCAL);
		wf._TCP_Local_Server_EN=0;							//Habilito el Servidor Local
		wf._data2SND[0]=0x00;//strcpy(wf._data2SND,"01;03;00;00;00;0A;C5;CD");//strcpy(wf._data2SND,"20;352;52#");
		wf._data2SND[1]=0x00;
		wf._data2SND[2]=0x00;
		wf._data2SND[3]=0x00;
		wf._data2SND[4]=0x00;
		wf._data2SND[5]=0x06;
		wf._data2SND[6]=0x01;
		wf._data2SND[7]=0x03;
		wf._data2SND[8]=0x00;//strcpy(wf._data2SND,"01;03;00;00;00;0A;C5;CD");//strcpy(wf._data2SND,"20;352;52#");
		wf._data2SND[9]=0x00;
		wf._data2SND[10]=0x00;
		wf._data2SND[11]=0x0A;
		wf._data2SND[12]=0x00;
		wf._data2SND[13]=0x33;
		wf._data2SND[14]=0x34;
		wf._data2SND[15]=0x35;
		wf._n_D2SND=12;
		wf._estado_conexion=100;//Si no se define no arranca	//wf._estado_conexion=1;					//Arranco en WiFi Desconectado
		wf._automatizacion=WF_CONNECT_TCP;//wf._automatizacion=WF_SEND;
		wf._DBG_EN=1;
		wf._NO_IP=1;
		wf.RESET_PORT=GPIOA;
		wf.RESET_PIN=GPIO_PIN_8;
	 //----------------------- WIFI ------------------------//

	 //---------------------- ModBUS -----------------------//
		ModBUS_Config(&mb_eth);		//ETHERNET como cliente TCP envía  ModBUS
		mb_eth._mode = CLIENTE;
		ModBUS_Config(&mb_wf);		//WIFI como servidor TCP, recibe comadno ModBUS
		mb_wf._mode = CLIENTE;
		ModBUS_F03_Assign(&mb_wf,3,0xAA55);
		pos_decimal[0]=1;
		pos_decimal[1]=1;
		pos_decimal[2]=1;
		pos_decimal[3]=1;
		pos_decimal[4]=1;
		pos_decimal[5]=1;
		pos_decimal[6]=1;
		pos_decimal[7]=1;
		pos_decimal[8]=1;
		pos_decimal[9]=1;
		pos_decimal[10]=1;
		pos_decimal[11]=1;
		pos_decimal[12]=1;
		pos_decimal[13]=1;
		pos_decimal[14]=1;
		pos_decimal[15]=1;

	 //---------------------- ModBUS -----------------------//
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  SysTick_Config(SystemCoreClock/1000);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

     ITM0_Write("\r\n INICIO OK\r\n",strlen("\r\n INICIO OK\r\n"));
     HAL_UART_Transmit_IT(&huart2,"\r\n INICIO OK \r\n",strlen("\r\n INICIO OK \r\n"));
	 HW_RESET(&wf);
	 if (wf._DBG_EN) ITM0_Write("\r\n RESET ESP8266 \r\n",strlen("\r\n RESET ESP8266 \r\n"));
	 HAL_UART_Receive_IT(&huart1,(uint8_t *)UART_RX_byte,1);
	 if (ETH_DBG_EN)ITM0_Write("\r\n SET-UP W5500 \r\n",strlen("\r\n SET-UP W5500 \r\n"));


     SPI_ETH_PHY_RESET(&ETH);
     HAL_Delay(800);
     eth_init(&ETH);
     SPI_ETH_SNIFF(&ETH_SPY, &ETH);
     HAL_Delay(800);
   	 eth_socket_init(&ETH,S0_REG);
   	 SPI_ETH_SNIFF(&ETH_SPY, &ETH);
	 uint16_t b=0;
	 uint8_t spi_Data[2];
	 b = SPI_ETH_REG(&ETH, S_TX_WR, S0_REG ,SPI_READ, spi_Data,2);
	 eth_wr_SOCKET_MODE(&ETH,S0_REG,MODE_TCP);
   	 SPI_ETH_REG(&ETH, S_DIPR, S0_REG,SPI_WRITE, ETH.S_DIPR,4);									// client
   	 ITM0_Write("\r\nETH-W5500-SOCK0 TCP REMOTE PORT TO CONNECT\r\n",strlen("\r\nETH-W5500-SOCK0 TCP REMOTE PORT TO CONNECT\r\n"));									// client
   	 SPI_ETH_REG(&ETH, S_DPORT, S0_REG,SPI_WRITE, ETH.S_DPORT,2);									// client
   	 ITM0_Write("\r\nETH-W5500-SOCK0 TCP PORT SET\r\n",strlen("\r\nETH-W5500-SOCK0 TCP PORT SET\r\n"));						//same for server and client
   	 SPI_ETH_SNIFF(&ETH_SPY, &ETH);
   	 eth_wr_SOCKET_CMD(&ETH, S0_REG,OPEN );

   	 HAL_Delay(1000);
   	 SPI_ETH_SNIFF(&ETH_SPY, &ETH);

	 //SPI_READ_EN=1;
   	 //------------------------ RUN ESP -----------------------------------//
     if(ESP8266_HW_Init(&huart1)==1)
     {
		  ESP_HW_Init=1;
		  if (wf._DBG_EN) ITM0_Write("\r\n ESP HW Init OK\r\n",strlen("\r\n ESP HW Init OK\r\n"));
		  HAL_UART_Transmit_IT(&huart2,"\r\n ESP HW Init OK\r\n",strlen("\r\n ESP HW Init OK\r\n"));
     }
     else
     {

    	 HW_RESET(&wf);
		 ESP_REinit=0;
			  if(ESP8266_HW_Init(&huart1)==1)
				  {
					  ESP_HW_Init=1;
					  if (wf._DBG_EN) ITM0_Write("\r\n ESP HW Init OK\r\n",strlen("\r\n ESP HW Init OK\r\n"));
					  HAL_UART_Transmit_IT(&huart2,"\r\n ESP HW Init2 OK\r\n",strlen("\r\n ESP HW Init2 OK\r\n"));
				  }
				  else
				  {
					  ESP_HW_Init=0;
					  if (wf._DBG_EN)  ITM0_Write("\r\n ESP HW Init Fail\r\n",strlen("\r\n ESP HW Init Fail\r\n"));
					  HAL_UART_Transmit_IT(&huart2,"\r\n ESP HW Fail\r\n",strlen("\r\n ESP HW Fail\r\n"));
				  }
     }
     //------------------------ RUN ESP -----------------------------------//

     HAL_Delay(1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		 SPI_READ_EN=1;
		  //----------------INSTRUCCIONS POR PUERTO SERIE---------------------

		  //----------------INSTRUCCIONS POR PUERTO SERIE---------------------

		  /**************[ INICIO PIDO ENVIAR DATOS ]**************/





		  	  if (ESP_HW_Init==1)
		  	  {
		  			if((WF_SND_FLAG==1)&&(wf._TCP_Local_Server_EN==0)&&(wf._estado_conexion>=609)&&(ETH.S_data_available))
		  			{	ETH.S_data_available=0;
		  				wf_snd_flag_ticks=0;
		  				WF_SND_FLAG=0;
		  				HAL_UART_Transmit_IT(&huart2,"\r\n ENTER TO SEND DATA \r\n",strlen("\r\n ENTER TO SEND DATA \r\n"));
		  				HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		  				ITM0_Write("\r\n#RED#WF-SE GENERA INFO Y ENVIA\r\n",strlen("\r\n#RED#WF-SE GENERA INFO Y ENVIA\r\n"));

		  				for(uint8_t i=0;i<=16;i++)
		  				{
		  					datos[i]=ModBUS_F03_Read(&mb_eth,i);
		  				}

		  				if(httpPOST2(ENDPOINT, SERVER_IP,PORT,&datos,&pos_decimal,16,TEST_2,post, body, 512))
		  				{
  							CopiaVector(wf._data2SND,post,strlen(post),0,'A');
  							wf._n_D2SND=strlen(post);
  							if(wf._automatizacion < WF_SEND)		// Send only with automation sent diasabled
  							{
  								EnviarDatos(&wf);
  								wf._estado_conexion=TCP_SND_EN_CURSO;
  							}
		  				}

		  				/*if( httpPOST(	ENDPOINT, SERVER_IP,PORT,
		  								ModBUS_F03_Read(&mb_eth,0),
		  								ModBUS_F03_Read(&mb_eth,1),
		  								ModBUS_F03_Read(&mb_eth,2),
		  								ModBUS_F03_Read(&mb_eth,3),
		  								ModBUS_F03_Read(&mb_eth,4),
		  								ModBUS_F03_Read(&mb_eth,5),
		  								ModBUS_F03_Read(&mb_eth,6),
		  								ModBUS_F03_Read(&mb_eth,7),
		  								ModBUS_F03_Read(&mb_eth,8),
										ModBUS_F03_Read(&mb_eth,9),
										TEST_2,//ModBUS_F03_Read(&mb_eth,9),TEPELCO,
		  								post, body, 512))

		  				{
		  							CopiaVector(wf._data2SND,post,strlen(post),0,'A');
		  							wf._n_D2SND=strlen(post);
		  							if(wf._automatizacion < WF_SEND)		// Send only with automation sent diasabled
		  							{
		  								EnviarDatos(&wf);
		  								wf._estado_conexion=TCP_SND_EN_CURSO;
		  							}
		  				}*/
		  			}
		  	  }
		  /**************[ FIN PIDO ENVIAR DATOS ]**************/

		  		if ((FLAG_UART1==1)||(FLAG_TIMEOUT==1))  //Si recibí datos o me fui por TimeOUT
		  		{
		  			if(FLAG_UART1==1)
		  				{
		  					CopiaVector(wf._uartRCVD,UART_RX_vect_hld,UART_RX_items,1,CMP_VECT);
		  					FLAG_UART1=0;

		  						if (error_rxdata==3)
		  						{
		  							error_rxdata=0;
		  						}
		  						if (error_rxdata==1)
		  						{
		  							error_rxdata=5;
		  							error_rxdata=0;
		  						}
		  				}
		  			if(FLAG_TIMEOUT==1)
		  					{
		  						FLAG_TIMEOUT=0;
		  					}

		  			if (ESP_HW_Init==1) //Si el módulo se inició correctamente
		  				{
		  					/*************** Copio y proceso info recibida ***************/
		  					wf._n_orig=UART_RX_items;
		  					CopiaVector(wf._uartRCVD,UART_RX_vect_hld,UART_RX_items,1,CMP_VECT);
		  					resultado=AT_ESP8266_ND(&wf);

		  					/*************** Si recibo datos y estan correctos me fijo que son ***************/

		  					if ((wf._new_data_rcv==1)&&(wf._estado_rcv_data==99))
		  					{

		  						CopiaVector(mb_wf._MBUS_RCVD,wf._dataRCV,wf._n_dataRCV,0,'A');
		  						mb_wf._n_MBUS_RCVD=wf._n_dataRCV;

		  						ModBUS(&mb_wf);

		  						CopiaVector(wf._data2SND,mb_wf._MBUS_2SND,mb_wf._n_MBUS_2SND,0,'A');
		  						wf._n_D2SND=mb_wf._n_MBUS_2SND;
		  						wf._new_data_rcv=0;//
		  						wf._send_data=1;
		  					}else
		  						{
		  							// DATA ERRONEA NO SE PROCESA
		  						}
		  					}

		  		}

		  		if (ESP_HW_Init==1) //Si el módulo se inició correctamente
		  			{
		  				conexion=WiFi_Conn_ND(&wf,&huart1,1);	//Tiene que ir en el main el chequeo es constante

		  			}
		  		if (esp_restart==1) //WRNNG Hardcoded RESET WIFI
		  			{

		  				HAL_UART_Transmit_IT(&huart2,"\r\n ESP HW Resetting \r\n",strlen("\r\n ESP HW Resetting \r\n"));
		  				HW_RESET(&wf);
		  				HAL_Delay(5000);//210419
		  				esp_restart=0;
		  			}
		  		if(eth_reset==1)
		  		{

		  		  SPI_ETH_PHY_RESET(&ETH);
		  		  HAL_UART_Transmit_IT(&huart2,"\r\n PHY RESET \r\n",strlen("\r\n PHY RESET \r\n"));
		  	      HAL_Delay(800);
		  	      eth_init(&ETH);
		  	      HAL_UART_Transmit_IT(&huart2,"\r\n ETH INIT \r\n",strlen("\r\n ETH INIT \r\n"));
		  	      SPI_ETH_SNIFF(&ETH_SPY, &ETH);
		  	      HAL_Delay(800);
		  	   	  eth_socket_init(&ETH,S0_REG);
		  	   	  HAL_UART_Transmit_IT(&huart2,"\r\n ETH SOCKET INIT \r\n",strlen("\r\n ETH SOCKET INIT \r\n"));

		  		 uint16_t b=0;
		  		 uint8_t spi_Data[2];
		  		 b = SPI_ETH_REG(&ETH, S_TX_WR, S0_REG ,SPI_READ, spi_Data,2);

		  		 eth_wr_SOCKET_MODE(&ETH,S0_REG,MODE_TCP);
		  	   	 SPI_ETH_REG(&ETH, S_DIPR, S0_REG,SPI_WRITE, ETH.S_DIPR,4);									// client
		  	   	 ITM0_Write("\r\nETH-W5500-SOCK0 TCP REMOTE PORT TO CONNECT\r\n",strlen("\r\nETH-W5500-SOCK0 TCP REMOTE PORT TO CONNECT\r\n"));									// client
		  	   	 SPI_ETH_REG(&ETH, S_DPORT, S0_REG,SPI_WRITE, ETH.S_DPORT,2);									// client
		  	   	 ITM0_Write("\r\nETH-W5500-SOCK0 TCP PORT SET\r\n",strlen("\r\nETH-W5500-SOCK0 TCP PORT SET\r\n"));						//same for server and client

		  	   	 HAL_Delay(800);
		  	   	  eth_wr_SOCKET_CMD(&ETH, S0_REG ,OPEN );
		  	   	  HAL_UART_Transmit_IT(&huart2,"\r\n ETH OPEN \r\n",strlen("\r\n ETH OPEN \r\n"));
		  	   	  ETH.ETH_WDG=0;
		  	   	  eth_reset=0;
		  		}

	  }




  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 100;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 100;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OnePulse_Init(&htim2, TIM_OPMODE_SINGLE) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_INACTIVE;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PCB_LED_GPIO_Port, PCB_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SPI1_NSS_Pin|WiFi_EN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI_RST_GPIO_Port, SPI_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : PCB_LED_Pin */
  GPIO_InitStruct.Pin = PCB_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PCB_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY_BTN_Pin */
  GPIO_InitStruct.Pin = KEY_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(KEY_BTN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI1_NSS_Pin */
  GPIO_InitStruct.Pin = SPI1_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI1_NSS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_RST_Pin */
  GPIO_InitStruct.Pin = SPI_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : WiFi_EN_Pin */
  GPIO_InitStruct.Pin = WiFi_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(WiFi_EN_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */


int ITM0_Write( char *ptr, int len)
{
 int DataIdx;

  for(DataIdx=0; DataIdx<len; DataIdx++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
}

void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
	unsigned char text[512];
	int ns=0;
	ms_ticks++;	//100 ms

	if(wf._estado_conexion!=ESPstate)
	{
		ESPstate=wf._estado_conexion;
		ns=INTOA(ESPstate,text);
		HAL_UART_Transmit_IT(&huart2,&text,ns);

	}

	ESP_ticks++;
	if(mb_eth._w_answer) MB_TOUT_ticks++;
	if ( mb_eth._w_answer && (mb_eth._timeout < MB_TOUT_ticks))
		{
			mb_eth._w_answer=0;
			MB_TOUT_ticks=0;
		}

// ENVIO DATOS LoRa ---------------------------------------------------------------//

/*	if(lr.tmr_dly_en==1)
	{
		lr.tmr_dlyCNT++;
		if(lr.tmr_dlyCNT > lr.tmr_dly)
		{
			lr.tmr_dly_ON=1;
			lr.tmr_dly_en=0;
		}
	}*/
// ENVIO DATOS LoRa ---------------------------------------------------------------//
// ENVIO DATOS WF ---------------------------------------------------------------//

	if((wf._estado_conexion==609 || wf._estado_conexion==700)&&(wf._TCP_Local_Server_EN==0))  wf_snd_flag_ticks++;

	if(wf_snd_flag_ticks>= 5000 && wf._ejecucion!=1 && wf._TCP_Local_Server_EN==0)		 	  WF_SND_FLAG=1;					//230517 wf_snd_flag_ticks>= 2000

// ENVIO DATOS WF ----------------------------------- ---------------------------//

/**********************[ INICIO - EHTERNET WDG ] **********************/

	if(ETH.S_status == 0)
	{
		ETH.ETH_WDG++;
		if (ETH.ETH_WDG>=64000)
		{
			eth_reset=1;
			ETH.ETH_WDG=64000;
		}
	}

/**********************[ FIN 	- EHTERNET WDG ] **********************/

if (ms_ticks==100)//(ms_ticks==250)//(ms_ticks==50)
  {
	  dbgn++;
	  ms_ticks=0;
	  min_ticks++;


	  	if(MBUS_ticks==360) MBUS_ticks=0;

	  	if (asc==0)  MBUS_ticks++;
	  	if (MBUS_ticks==100) asc=1;
	  	if (asc==1) MBUS_ticks--;
	  	if (MBUS_ticks==0) asc=0;


	  //HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
	  if(spi_no_debug)
	  	  {
	  if(SPI_READ_EN)
	  {
	     ETH.S_status=eth_rd_SOCKET_STAT(&ETH,S0_REG);  //este era el bardo

		  switch(ETH.S_status)	//Check Socket status
	     {
			 case SOCK_CLOSED :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_CLOSED \r\n",strlen("\r\nS0_SOCK_CLOSED \r\n"));
					 //if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_CLOSED \r\n",strlen("\r\n S0_SOCK_CLOSED \r\n"));
					 //eth_wr_SOCKET_CMD(&ETH, S0_REG ,OPEN );
					 if(ETH.ETH_WDG >= 15000)
					 {
						  eth_reset=1;
						 //ETH.ETH_WDG=1;
					 }



				 }
			 break;
			 case  SOCK_INIT :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_INIT \r\n",strlen("\r\nS0_SOCK_INIT \r\n"));
					 if(ETH.S_ENserver == 1)
					 {
						 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_INIT \r\n",strlen("\r\nS0_SOCK_INIT \r\n"));
						 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_INIT \r\n",strlen("\r\n S0_SOCK_INIT \r\n"));
							eth_wr_SOCKET_CMD(&ETH, S0_REG, LISTEN );
							ETH.ETH_WDG=0;
					 }
					 else
					 {

						    eth_wr_SOCKET_CMD(&ETH,S0_REG, CONNECT);																				//only for server
						 	if (ETH_DBG_EN)ITM0_Write("\r\nETH-W5500-CONNECT\r\n",strlen("\r\nETH-W5500-CONNECT\r\n"));
						 	if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n ETH-W5500-CONNECT \r\n",strlen("\r\n ETH-W5500-CONNECT \r\n"));
						 	ETH.ETH_WDG=0;
					 }

				 }
			 break;
			 case SOCK_LISTEN :
				 {
					 if (ETH_DBG_EN)ITM0_Write("\r\nS0_SOCK_LISTEN \r\n",strlen("\r\nS0_SOCK_LISTEN \r\n"));
					 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_LISTEN \r\n",strlen("\r\n S0_SOCK_LISTEN \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_SYNSENT :
				 {
					 if (ETH_DBG_EN)ITM0_Write("\r\nS0_SOCK_SYNSENT \r\n",strlen("\r\nS0_SOCK_SYNSENT \r\n"));
					 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_SYNSENT \r\n",strlen("\r\n S0_SOCK_SYNSENT \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_SYNRECV :
				 {
					 if (ETH_DBG_EN)ITM0_Write("\r\nS0_SOCK_SYNRECV \r\n",strlen("\r\nS0_SOCK_SYNRECV \r\n"));
					 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_SYNRECV \r\n",strlen("\r\n S0_SOCK_SYNRECV \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_ESTABLISHED :
				 {
					 if (ETH_DBG_EN)ITM0_Write("\r\nS0_SOCK_ESTABLISHED \r\n",strlen("\r\nS0_SOCK_ESTABLISHED \r\n"));
					 //if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_ESTABLISHED \r\n",strlen("\r\n S0_SOCK_ESTABLISHED \r\n"));
					 ETH.ETH_WDG=0;

					if (ETH.S_ENserver == 1)  // Si el puerto Ethernet actúa como server (Recibe datos conexión mas pedido mbus
					{

						S0_get_size = SPI_ETH_REG(&ETH, S_RX_RSR ,S0_REG, SPI_READ, spi_Data,2);//S0_get_size = SPI_ETH_REG(&ETH, S_RX_SZ_ADDR_BASEHH,S_RX_SZ_ADDR_BASEHL ,SPI_READ, spi_Data,2);
							if(S0_get_size != 0x00)
							{
								eth_rd_SOCKET_DATA(&ETH,S0_RX_BUFF,&rx_mem_pointer,S0_get_size); // read socket data
								SPI_ETH_WR_REG_16(&ETH,S_RX_RD0,rx_mem_pointer,S0_REG );		// write rx memory pointer
								eth_wr_SOCKET_CMD(&ETH,S0_REG,RECV);							// write command to execute
								while(eth_rd_SOCKET_CMD(&ETH,S0_REG))						// wait until end of command execution
								{}

								CopiaVector(mb_eth._MBUS_RCVD, ETH.data, S0_get_size, 0, 0 );
								mb_eth._n_MBUS_RCVD=S0_get_size;

								if(S0_get_size > 0)	{ ETH.S_data_available=1;}					//Flag data received

								if(ModBUS_Check(mb_eth._MBUS_RCVD, mb_eth._n_MBUS_RCVD))		//Ckecks ModBUS type data
								{
									ModBUS(&mb_eth);										//ModBUS protocol execution
									CopiaVector(ETH.data, mb_eth._MBUS_2SND, mb_eth._n_MBUS_2SND, 0, 0);
								}
								else
								{
									if (ETH_DBG_EN) ITM0_Write("\r\n NO MBUS \r\n",strlen("\r\n\r\n NO MBUS \r\n\r\n"));
									if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n NO MBUS \r\n",strlen("\r\n NO MBUS \r\n"));
								}

								send_size=mb_eth._n_MBUS_2SND;  //ModBUS data qty

								eth_wr_SOCKET_DATA(&ETH,S0_RX_BUFF, &tx_mem_pointer, send_size);	// write socket data

								SPI_ETH_WR_REG_16(&ETH,S_TX_WR,tx_mem_pointer,S0_REG);			// write tx memory pointer//SPI_ETH_WR_REG_16(&ETH,0x424,tx_mem_pointer,0);			// write tx memory pointer
								eth_wr_SOCKET_CMD(&ETH,S0_REG,SEND);							// write command to execute
								while(eth_rd_SOCKET_CMD(&ETH,S0_REG))						// wait until end of command execution
								{}

							}
					}
					else	// Puerto ethernet labura como esclavo, se conecta al server para pedir datos
					{

						if (mb_eth._w_answer==0)
						{
							//Si ya envié vuelvo a enviar

							ETH.data[0]=0x00;
							ETH.data[1]=0x00;
							ETH.data[2]=0x00;
							ETH.data[3]=0x00;
							ETH.data[4]=0x00;
							ETH.data[5]=0x06;
							ETH.data[6]=0x01;
							ETH.data[7]=0x03;
							ETH.data[8]=0x00;
							ETH.data[9]=0x00;
							ETH.data[10]=0x00;
							ETH.data[11]=0x0A;
							send_size=12;

							ModBUS_F03_Request(&mb_eth,0,16);
							CopiaVector(ETH.data, mb_eth._MBUS_2SND, 12, 0, 0 );


							eth_wr_SOCKET_DATA(&ETH,S0_TX_BUFF, &tx_mem_pointer, send_size);	// write socket data
						   	 SPI_ETH_SNIFF(&ETH_SPY, &ETH);
							SPI_ETH_WR_REG_16(&ETH,S_TX_WR,tx_mem_pointer,S0_REG);			// write tx memory pointer
						   	 SPI_ETH_SNIFF(&ETH_SPY, &ETH);
							eth_wr_SOCKET_CMD(&ETH,S0_REG,SEND);							// write command to execute
						   	 SPI_ETH_SNIFF(&ETH_SPY, &ETH);
							uint16_t read=0;
							//while(read != 16)//while(eth_rd_SOCKET_CMD(&ETH,0))						// wait until end of command execution
							//{
								read=SPI_ETH_REG(&ETH, S_IR,S0_REG,SPI_READ, ETH.GAR,1);
							//}

							mb_eth._w_answer=1;	// Waiting answer flag_w_answer=1;	// Waiting answer flag
							MB_TOUT_ticks=0;	// restart counting
							if (ETH_DBG_EN) ITM0_Write("\r\n SENT MBUS REQ \r\n",strlen("\r\n\r\n SENT MBUS REQ \r\n\r\n"));
							if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n SENT MBUS REQ \r\n",strlen("\r\n SENT MBUS REQ \r\n"));
						}
						else
						{
						S0_get_size = SPI_ETH_REG(&ETH, S_RX_RSR ,S0_REG ,SPI_READ, spi_Data,2);
						//_w_answer=0;	// Waiting answer flag
						if(S0_get_size != 0x00)
							{
								eth_rd_SOCKET_DATA(&ETH,S0_RX_BUFF,&rx_mem_pointer,S0_get_size); // read socket data
								SPI_ETH_WR_REG_16(&ETH,S_RX_RD0,rx_mem_pointer,S0_REG);		// write rx memory pointer
								eth_wr_SOCKET_CMD(&ETH,S0_REG,RECV);							// write command to execute
								/*while(eth_rd_SOCKET_CMD(&ETH,S0_REG))						// wait until end of command execution
								{}*/
								if (ETH_DBG_EN) ITM0_Write("\r\n RCVD DATA \r\n",strlen("\r\n RCVD DATA \r\n"));
								if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n RCVD DATA \r\n",strlen("\r\n RCVD DATA \r\n"));
								CopiaVector(mb_eth._MBUS_RCVD, ETH.data, S0_get_size, 0, 0 );
								mb_eth._n_MBUS_RCVD=S0_get_size;

								if(S0_get_size > 0)	{ ETH.S_data_available=1;}

								if(ModBUS_Check(mb_eth._MBUS_RCVD, mb_eth._n_MBUS_RCVD))		//Ckecks ModBUS type data
									{
										mb_eth._w_answer=0;  									//Si el mensaje recibido ya es modbus digo que ya recibi
										MB_TOUT_ticks=0;
										ModBUS(&mb_eth);										//ModBUS protocol execution
										//CopiaVector(ETH.data, mb_eth._MBUS_2SND, mb_eth._n_MBUS_2SND, 0, 0);
										CopiaVector(ETH.swap, mb_eth._MBUS_RCVD, mb_eth._n_MBUS_RCVD, 0, 0);
										CopiaVector(mb_wf._Holding_Registers, mb_eth._Holding_Registers, 64, 0, 0);
										if (ETH_DBG_EN) ITM0_Write("\r\n RCVD MBUS REQ \r\n",strlen("\r\n\ RCVD MBUS REQ \r\n"));
										if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n RCVD MBUS REQ \r\n",strlen("\r\n RCVD MBUS REQ \r\n"));
									}
									else
										{
										if (ETH_DBG_EN) ITM0_Write("\r\n NO MBUS \r\n",strlen("\r\n NO MBUS \r\n"));
										if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n NO MBUS \r\n",strlen("\r\n NO MBUS \r\n"));
										}


							}
						}
					}
				 }
			 break;
			 case SOCK_FIN_WAIT :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_FIN_WAIT \r\n",strlen("\r\nS0_SOCK_FIN_WAIT \r\n"));
					 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_FIN_WAIT \r\n",strlen("\r\n S0_SOCK_FIN_WAIT \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_CLOSING :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_CLOSING \r\n",strlen("\r\nS0_SOCK_CLOSING \r\n"));
					 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_CLOSING \r\n",strlen("\r\n S0_SOCK_CLOSING \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case  SOCK_TIME_WAIT :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_TIME_WAIT \r\n",strlen("\r\nS0_SOCK_TIME_WAIT \r\n"));
					 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n nS0_SOCK_TIME_WAIT \r\n",strlen("\r\n nS0_SOCK_TIME_WAIT \r\n"));
					eth_wr_SOCKET_CMD(&ETH,S0_REG, DISCON );
					while( SPI_ETH_REG(&ETH,S_CR ,S0_REG,SPI_READ, spi_Data,1))//while( SPI_ETH_REG(&ETH, S_CR_ADDR_BASEH,S_CR_ADDR_BASEL ,SPI_READ, spi_Data,1))
					{}
					ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_CLOSE_WAIT :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\nS0_SOCK_CLOSE_WAIT \r\n",strlen("\r\nS0_SOCK_CLOSE_WAIT \r\n"));
					 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_CLOSE_WAIT \r\n",strlen("\r\n S0_SOCK_CLOSE_WAIT \r\n"));
					eth_wr_SOCKET_CMD(&ETH,S0_REG,DISCON );
					while( SPI_ETH_REG(&ETH,S_CR,S0_REG,SPI_READ, spi_Data,1))
					{}
					ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_LAST_ACK :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\n S0_SOCK_LAST_ACK \r\n",strlen("\r\n S0_SOCK_LAST_ACK \r\n"));
					 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_LAST_ACK \r\n",strlen("\r\n S0_SOCK_LAST_ACK \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_UDP :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\n S0_SOCK_UDP \r\n",strlen("\r\n S0_SOCK_UDP \r\n"));
					 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_UDP \r\n",strlen("\r\n S0_SOCK_UDP \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case  SOCK_IPRAW :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\n S0_SOCK_IPRAW \r\n",strlen("\r\n S0_SOCK_IPRAW \r\n"));
					 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_IPRAW \r\n",strlen("\r\n S0_SOCK_IPRAW \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case  SOCK_MACRAW :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\n S0_SOCK_MACRAW \r\n",strlen("\r\n S0_SOCK_MACRAW \r\n"));
					 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_MACRAW \r\n",strlen("\r\n S0_SOCK_MACRAW \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;
			 case SOCK_PPOE :
				 {
					 if (ETH_DBG_EN) ITM0_Write("\r\n S0_SOCK_PPOE \r\n",strlen("\r\n S0_SOCK_PPOE \r\n"));
					 if (ETH_DBG_SER_EN) HAL_UART_Transmit_IT(&huart2,"\r\n S0_SOCK_PPOE \r\n",strlen("\r\n S0_SOCK_UDP \r\n"));
					 ETH.ETH_WDG=0;
				 }
			 break;

			 default:
				 {

				 }
	     }
	  }
	  }else
	  	  {
		  //ETH.operacion=SPI_READ;
		  //ETH.TX[3]=0x00;
		  SPI_ETH(&ETH);
	  	  }
	  if(min_ticks==2)//if(min_ticks==10)
		  {
		  	  min_ticks=0;  /* SETEO CADA 2 min*/
		  }
  }

if(EN_USART1_TMR==1) USART1_ticks++;

if(USART1_ticks>=2)//if(USART1_ticks>=10)
{
	USART1_ticks=0;
	FLAG_USART1=1;
	EN_USART1_TMR=0;
	items_rx=uart1pass;
	uart1pass=0;
}

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
	if(wf._estado_conexion==4)//if((wf._estado_conexion!=1)&&(wf._estado_conexion!=2)&&(resultado!=20)&&(resultado!=24)) //Solo cuento cuando no estahaciendo otra cosa
	{
		ticks++;
	}
	else
	{
		ticks=0;
	}

	if(wf._ejecucion==1)
		{
			if (FLAG_TIMEOUT!=1)
			{
				if(wf._instruccion!=2) wf._ticks++;//-----------------------Solo cuento una vez reconcido el timeout, cuando entro al timeout no cuento
				if(wf._instruccion==2) wf._ticks2++;
			}


			if ((wf._instruccion!=2)&&(wf._ticks > 5500)) //if (wf._ticks > 5000)
			{
				FLAG_TIMEOUT=1;
				if(huart1.Instance->CR1 == 0x200C)  //--------------------Evito error UART colgado
				{
					HAL_UART_Receive_IT(&huart1,(uint8_t *)UART_RX_byte,1);
					EN_UART1_TMR=0; //OBS-VER Para que me vuelva a habilitar el timer
				}
				//wf._ticks=0;
			}
			if ((wf._instruccion==2)&&(wf._ticks2 > 20500)) //if (wf._ticks > 5000)
			{
				FLAG_TIMEOUT=1;
				if(huart1.Instance->CR1 == 0x200C)  //--------------------Evito error UART colgado
				{
					HAL_UART_Receive_IT(&huart1,(uint8_t *)UART_RX_byte,1);
					EN_UART1_TMR=0; //OBS-VER Para que me vuelva a habilitar el timer
				}
				//wf._ticks=0;
			}

		}
		else
		{
			wf._ticks=0;
		}
  /* USER CODE END SysTick_IRQn 1 */
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *ERRUART)

{
	if(ERRUART->Instance==USART1)
	{
		 volatile int aore=0;
		 volatile int bore=0;

		// if ( UART_FLAG_ORE == HAL_UART_GetError(huart1))
		//{
		//Al leer los registros de esta forma SR y luego DR se resetean los errores de Framing Noise y Overrun FE NE ORE
		//}
			 wf._debug_count9++;
			aore=ERRUART->Instance->SR;
			bore=ERRUART->Instance->DR;


		 HAL_UART_Transmit_IT(&huart2,"\r\n ERROR HW UARRT1 \r\n",strlen("\r\n ERROR HW UARRT1 \r\n"));
		 HAL_UART_DeInit(ERRUART);
		 MX_USART1_UART_Init();
		 HAL_UART_Receive_IT(ERRUART,(uint8_t *)UART_RX_byte,1);
	}
	if(ERRUART->Instance==USART2)
	{
		 volatile int aore=0;
		 volatile int bore=0;

		// if ( UART_FLAG_ORE == HAL_UART_GetError(huart1))
		//{
		//Al leer los registros de esta forma SR y luego DR se resetean los errores de Framing Noise y Overrun FE NE ORE
		//}
			 wf._debug_count9++;
			aore=ERRUART->Instance->SR;
			bore=ERRUART->Instance->DR;


		//HAL_UART_Transmit_IT(&huart5,"U4",strlen("U4"));
		 HAL_UART_DeInit(ERRUART);
		 MX_USART2_UART_Init();
		 HAL_UART_Receive_IT(ERRUART,(uint8_t *)UART_RX_byte,1);
	}
}

void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *htim2)
{

		wf._debug_count10++;

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *INTSERIE)
{

// WiFi	USART 1 TIMER2
	if(INTSERIE->Instance==USART1)
		 {
			UART_RX_vect[UART_RX_pos]=UART_RX_byte[0];
			UART_RX_pos++;
			if(UART_RX_pos>=512) UART_RX_pos=512;
			HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);//HAL_TIM_Base_Start_IT(&htim7);	//Habilito el timer
			TIM2->CNT=1;
			EN_UART1_TMR=1;	//Habilito Timeout de software
			HAL_UART_Receive_IT(INTSERIE,(uint8_t *)UART_RX_byte,1);
		 }
 }

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *TIMER)
{

		if(TIMER->Instance==TIM2)
			{
				 HAL_TIM_OC_Stop_IT(TIMER, TIM_CHANNEL_1); //Paro el timer
				 FLAG_UART1=1;
				 EN_UART1_TMR=0;
				 UART_RX_items=UART_RX_pos;
				 UART_RX_pos=0;
				 UART_RX_vect[512]='\0'; //Finalizo el vector a la fuerza ya que recibo hasta 124
				 CopiaVector(UART_RX_vect_hld,UART_RX_vect,UART_RX_items,1,CMP_VECT);
				 HAL_UART_Receive_IT(&huart1,(uint8_t *)UART_RX_byte,1); //Habilito le recepcón de puerto serie al terminar
				 if (wf._DBG_EN==1)
				 {
					 ITM0_Write((uint8_t *)UART_RX_vect_hld,UART_RX_items);
				 }
		}
}

uint8_t ESP8266_HW_Init(UART_HandleTypeDef *SerialPort) //Devuelve 1 si reinició OK, y 0 si no
{
	  do{
		  HAL_UART_Transmit(SerialPort, "AT+RESTORE\r\n",strlen("AT+RESTORE\r\n"),100);
		  HAL_Delay(500);

		  wf._n_fcomp=strlen("ready");
		  wf._n_orig=UART_RX_items;

		  while(FT_String_ND(UART_RX_vect_hld,&wf._n_orig,"ready",&wf._n_fcomp,wf._uartRCVD_tok,&wf._n_tok,&ntestc,&wf._id_conn,510,FIND)!=1)
		  {
			  	  wf._n_orig=UART_RX_items;
			  	  if (ESP_ticks>=5000)
			  		 {
			  		 ESP_InitF=1;
			  		 break;
			  		 }
		  }

		  if(ESP_InitF==0)	//Si encontró la sentencia anterior analizo la siguiente
		  {
			  wf._n_fcomp=strlen("ready");
			  wf._n_orig=UART_RX_items;
			  while(FT_String_ND(UART_RX_vect_hld,&wf._n_orig,"ready",&wf._n_fcomp,wf._uartRCVD_tok,&wf._n_tok,&ntestc,&wf._id_conn,510,FIND)!=1)
			  {
				  wf._n_orig=UART_RX_items;
				  if (ESP_ticks>=5000)
					 {
					 break;
					 }
			  }
		  }

		  if (ESP_ticks<5000)
		  {
			  ESP_REinit=10;
			  ESP_ticks=0;
		  }
		  else
		  {
			  ESP_REinit++;
			  ESP_ticks=0;
		  }

	  } while (ESP_REinit<=5);

	  if(ESP_REinit==10)
	  {
		  return(1);
	  }
	  else
	  {
		  return(0);
	  }
}
void BorrarVect(void)
{
	wf._uartRCVD[0]='\0';
	wf._uartRCVD[1]='\0';
	wf._n_orig=2;
}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

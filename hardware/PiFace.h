#pragma once

#include "DomoticzHardware.h"
#include "../main/RFXtrx.h"


#define CONFIG_NR_OF_PARAMETER_TYPES 10
#define CONFIG_NR_OF_PARAMETER_BOOL_TYPES 4
#define CONFIG_NR_OF_PARAMETER_PIN_TYPES 4

#define LEVEL			0
#define INV_LEVEL			1
#define TOGGLE_RISING	2
#define TOGGLE_FALLING 	3


using namespace std;

class CIOCount
{
   public:
	   CIOCount();
	   ~CIOCount();

	   int Update(unsigned long Counts);
	   unsigned long GetCurrent(void) const {return Current;};
	   unsigned long GetTotal(void) const {return Total;};
	   unsigned long GetLastTotal(void) const { return LastTotal; };
	   unsigned long GetRateLimit(void) const { return Minimum_Pulse_Period_ms; };
	   void SetCurrent(unsigned long NewCurValue) {Current=NewCurValue;};
	   void SetTotal(unsigned long NewTotalValue) { Total = NewTotalValue; };
	   void SetLastTotal(unsigned long NewTotalValue) { LastTotal = NewTotalValue; };
	   void SetRateLimit(unsigned long NewRateLimit) { Minimum_Pulse_Period_ms = NewRateLimit; };
	   void ResetCurrent(void) {Current=0;};
	   void ResetTotal(void) {
		   Total = 0;
		   LastTotal = 0;
	   };
	   bool ProcessUpdateInterval(unsigned long PassedTime_ms);
	   void SetUpdateInterval(unsigned long NewValue_ms);
	   unsigned long GetUpdateInterval(void) {return UpdateInterval_ms;};
	   bool Enabled;
	   bool InitialStateSent;

   private:
	   unsigned long Current;
	   unsigned long Total;
	   unsigned long LastTotal;
	   unsigned long UpdateInterval_ms;
	   unsigned long UpdateDownCount_ms;
	   unsigned long Minimum_Pulse_Period_ms;
	   boost::posix_time::ptime Last_Call;
	   boost::posix_time::ptime Cur_Call;
};

class CIOPinState
{
   public:
	   CIOPinState();
	   ~CIOPinState();

	   int Update(bool New);
	   int UpdateInterrupt(bool IntFlag,bool PinState);
	   int GetInitialState(void);
	   bool GetCurrent(void) const { return Current;}
	   bool Enabled;
	   int Id;
	   int Type;
	   CIOCount Count;
	   bool InitialStateSent;
	   unsigned char Direction;

   private:
	   bool Last;
	   bool Current;
	   bool Toggle;

};



class CIOPort
{
   public:
	   CIOPort();
	   ~CIOPort();
	   int Update(unsigned char New);
	   int UpdateInterrupt(unsigned char IntFlag,unsigned char PinState);
	   unsigned char GetCurrent(void) const { return Current;}
	   int GetDevId(void) const { return devId;}
	   bool IsDevicePresent(void) const { return Present;}
	   void Init(bool Available, unsigned char housecode, unsigned char initial_state);
       void SetID(int devID) { devId= devID;}
	   void ConfigureCounter(unsigned char Pin,bool Enable);
	   void *Callback_pntr;
	   CIOPinState  Pin[8];
	   unsigned int PortType;

   private:
	   tRBUF IOPinStatusPacket;
	   tRBUF IOPinCounterPacket;
	   unsigned char Last;
	   unsigned char Current;
	   int devId;
	   bool Present;
};

class CPiFace : public CDomoticzHardwareBase
{
public:
	explicit CPiFace(const int ID);
	~CPiFace();
	bool WriteToHardware(const char *pdata, const unsigned char length);
	void CallBackSendEvent(const unsigned char *pEventPacket, const unsigned int PacketLength);
	void CallBackSetPinInterruptMode(unsigned char devId,unsigned char pinID, bool Interrupt_Enable);

private:
	bool StartHardware();
	bool StopHardware();

	void Do_Work();
	void Do_Work_Queue();
	boost::shared_ptr<boost::thread> m_thread;
	boost::shared_ptr<boost::thread> m_queue_thread;

	volatile bool m_stoprequested;
	int m_InputSample_waitcntr;
	int m_CounterEdgeSample_waitcntr;

	int Init_SPI_Device(int Init);
	void Init_Hardware(unsigned char devId);
	int Read_Write_SPI_Byte(unsigned char *data, int len);
	int Read_MCP23S17_Register(unsigned char devId, unsigned char reg);
	int Write_MCP23S17_Register(unsigned char devId, unsigned char reg, unsigned char data);
	void Sample_and_Process_Inputs(unsigned char devId);
	void Sample_and_Process_Outputs(unsigned char devId);
	void Sample_and_Process_Input_Interrupts(unsigned char devId);
	void GetAndSetInitialDeviceState(unsigned char devId);


	int Detect_PiFace_Hardware(void);

	int m_fd;
	//int m_HwdID;

	CIOPort m_Inputs[4];
	CIOPort m_Outputs[4];

	bool m_DetectedHardware[4];

	//config file functions
	std::string & preprocess(std::string &s);
	std::string & trim(std::string &s);
	std::string & ltrim(std::string &s);
	std::string & rtrim(std::string &s);
	int LocateValueInParameterArray(std::string Parametername,const std::string *ParameterArray,int Items);
	int GetParameterString(std::string TargetString,const char * SearchStr, int StartPos,  std::string &Parameter);
	int LoadConfig(void);
	void LoadDefaultConfig(void);
	void AutoCreate_piface_config(void);

	void GetLastKnownValues(void);

	boost::mutex m_queue_mutex;
	std::vector<std::string> m_send_queue;
};

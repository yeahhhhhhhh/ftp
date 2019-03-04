#ifndef __CONFPARSEUTIL_H__
#define __CONFPARSEUTIL_H__

typedef struct ConfBoolSetting{
    const char* key;
    int *value;
}ConfBoolSetting;

typedef struct ConfUintSetting{
    const char* key;
    unsigned int *value;
}ConfUintSetting;


class ConfParseUtil {
    public:
      static int pasvEnable;
      static int portEnable;
      static unsigned int listenPort;
      static unsigned int maxClients;
      static unsigned int maxPerIp;
      static unsigned int connectTimeout;
      static unsigned int idleSessionTime;
      static unsigned int dataConnectionTimeout;
      static unsigned int localUmask;
      static unsigned int uploadMaxRate;
      static unsigned int downloadMaxRate;
	  ConfBoolSetting* confBoolSetting_;
	  ConfUintSetting* confUintSetting_;
    public:
	  ConfParseUtil ();
	  virtual ~ConfParseUtil ();
	 
	private:
	  void parseConfLoadSetting(const char* settingLine);
	  
};

#endif // __CONFPARSEUTIL_H__



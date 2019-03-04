#ifndef __PRIVPARENT_H__
#define __PRIVPARENT_H__

#include <linux/capability.h>

#include "Session.h"

class PrivParent {
    private:
	  /* data */
	  
    public:
	  PrivParent ();
	  virtual ~PrivParent ();
	
	private:
	  void getDataSock(Session* session);
	  void pasvActive(Session* session);
	  void pasvlisten(Session* session);
	  void pasvAccept(Session* session); 
	  int capset(cap_user_header_t hdrp, const cap_user_data_t datap);
	public:
	  void handleParent(Session *session);
	  
};

#endif // __PRIVPARENT_H__


/*_############################################################################
  _## 
  _##  notifyqueue.cpp  
  _##
  _##  SNMP++v3.2.24
  _##  -----------------------------------------------
  _##  Copyright (c) 2001-2009 Jochen Katz, Frank Fock
  _##
  _##  This software is based on SNMP++2.6 from Hewlett Packard:
  _##  
  _##    Copyright (c) 1996
  _##    Hewlett-Packard Company
  _##  
  _##  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  _##  Permission to use, copy, modify, distribute and/or sell this software 
  _##  and/or its documentation is hereby granted without fee. User agrees 
  _##  to display the above copyright notice and this license notice in all 
  _##  copies of the software and any documentation of the software. User 
  _##  agrees to assume all liability for the use of the software; 
  _##  Hewlett-Packard and Jochen Katz make no representations about the 
  _##  suitability of this software for any purpose. It is provided 
  _##  "AS-IS" without warranty of any kind, either express or implied. User 
  _##  hereby grants a royalty-free license to any and all derivatives based
  _##  upon this software code base. 
  _##  
  _##  Stuttgart, Germany, Fri May 29 22:35:14 CEST 2009 
  _##  
  _##########################################################################*/
/*===================================================================

  Copyright (c) 1999
  Hewlett-Packard Company

  ATTENTION: USE OF THIS SOFTWARE IS SUBJECT TO THE FOLLOWING TERMS.
  Permission to use, copy, modify, distribute and/or sell this software
  and/or its documentation is hereby granted without fee. User agrees
  to display the above copyright notice and this license notice in all
  copies of the software and any documentation of the software. User
  agrees to assume all liability for the use of the software; Hewlett-Packard
  makes no representations about the suitability of this software for any
  purpose. It is provided "AS-IS" without warranty of any kind,either express
  or implied. User hereby grants a royalty-free license to any and all
  derivatives based upon this software code base.

      N O T I F Y Q U E U E . C P P

      CNotifyEventQueue CLASS DEFINITION

      COPYRIGHT HEWLETT PACKARD COMPANY 1999

      INFORMATION NETWORKS DIVISION

      NETWORK MANAGEMENT SECTION

      DESIGN + AUTHOR:        Tom Murray

      DESCRIPTION:
        Queue for holding callback associated with user defined
        timeouts

=====================================================================*/
char notifyqueue_version[]="#(@) SNMP++ $Id: notifyqueue.cpp 352 2009-02-03 21:21:56Z katz $";

//-----[ includes ]----------------------------------------------------
#include <errno.h>

#if defined (CPU) && CPU == PPC603
#include <sockLib.h> 
#endif

#if defined(__APPLE__)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

//----[ snmp++ includes ]----------------------------------------------

#include "snmp_pp/config_snmp_pp.h"
#include "snmp_pp/v3.h"
#include "snmp_pp/notifyqueue.h" // queue for holding sessions waiting for async notifications
#include "snmp_pp/eventlistholder.h"
#include "snmp_pp/uxsnmp.h"
#include "snmp_pp/snmperrs.h"
#include "snmp_pp/pdu.h"
#include "snmp_pp/log.h"

#ifdef SNMP_PP_NAMESPACE
namespace Snmp_pp {
#endif

//--------[ externs ]---------------------------------------------------
extern int receive_snmp_notification(SnmpSocket sock, Snmp &snmp_session,
                                     Pdu &pdu, SnmpTarget **target);

//-----[ macros ]------------------------------------------------------
// should be in snmp.h...
#define SNMP_PORT 161	      // standard port # for SNMP
#define SNMP_TRAP_PORT 162    // standard port # for SNMP traps

#ifdef WIN32
#define close closesocket
#elif defined(_AIX)
#include <unistd.h>
#endif

//----[ CNotifyEvent class ]------------------------------------------------

CNotifyEvent::CNotifyEvent(Snmp *snmp,
			   const OidCollection &trapids,
			   const TargetCollection &targets,
			   const AddressCollection &addresses)
  : m_snmp(snmp)
{
  // create new collections using parms passed in
  notify_ids       = new OidCollection(trapids);
  notify_targets   = new TargetCollection(targets);
  notify_addresses = new AddressCollection(addresses);
}

CNotifyEvent::~CNotifyEvent()
{
  // free up local collections
  if (notify_ids)       { delete notify_ids;       notify_ids       = 0; }
  if (notify_targets)   { delete notify_targets;   notify_targets   = 0; }
  if (notify_addresses) { delete notify_addresses; notify_addresses = 0; }
}

int CNotifyEvent::notify_filter(const Oid &trapid, SnmpTarget &target) const
{
  int target_count, has_target = FALSE, target_matches = FALSE;
  int trapid_count, has_trapid = FALSE, trapid_matches = FALSE;
  GenAddress targetaddr, tmpaddr;

  // figure out how many targets, handle empty case as all targets
  if ((notify_targets) && (target_count = notify_targets->size()))
  {
    SnmpTarget *tmptarget = 0;
    has_target = TRUE;

    target.get_address(targetaddr);

    if (targetaddr.valid()) {
      // loop through all targets in the collection
      SnmpTarget::target_type target_type = target.get_type();
      SnmpTarget::target_type tmptarget_type;

      for ( int x = 0; x < target_count; x++)       // for all targets
      {
	if (notify_targets->get_element(tmptarget, x))
	  continue;

	tmptarget->get_address(tmpaddr);
	if ((tmpaddr.valid())) {
          int addr_equal = 0;

          /* check for types of Address */
          if ((tmpaddr.get_type() == Address::type_ip) &&
              (targetaddr.get_type() == Address::type_udp))
          {
            /* special case that works for UdpAddress == IpAddress */
            IpAddress ip1(targetaddr);
            IpAddress ip2(tmpaddr);

            addr_equal = (ip1.valid() && ip2.valid() && (ip1 == ip2));
          }
          else
          {
            addr_equal = (targetaddr == tmpaddr);
          }

          if (addr_equal) {
            tmptarget_type = tmptarget->get_type();
            if (target_type == SnmpTarget::type_utarget) {
              // target is a UTarget
              if (tmptarget_type == SnmpTarget::type_utarget) {
                // both are UTarget
                if ((((UTarget*)(&target))->get_security_name() ==
                     ((UTarget*)tmptarget)->get_security_name()) &&
                    (((UTarget*)(&target))->get_security_model() ==
                     ((UTarget*)tmptarget)->get_security_model())) {
		  target_matches = TRUE;
                  break;
                }
              }
              else
                if (tmptarget_type == SnmpTarget::type_ctarget)
                  // in case utarget is used with v1 or v2:
                  if ((tmptarget->get_version() == target.get_version()) &&
                      (((UTarget*)(&target))->get_security_name() ==
                       OctetStr(((CTarget*)tmptarget)->
                                get_readcommunity()))) {
                    target_matches = TRUE;
                    break;
                  }
            }
            else {
              if (target_type == SnmpTarget::type_ctarget) {
                // target is a CTarget
                if (tmptarget_type == SnmpTarget::type_ctarget) {
                  // both are CTarget
                  if (!strcmp(((CTarget*)(&target))->get_readcommunity(),
                              ((CTarget*)tmptarget)->get_readcommunity())) {
                    target_matches = TRUE;
                    break;
                  }
                }
                else
                  if (tmptarget_type == SnmpTarget::type_utarget) {
                    if ((tmptarget->get_version() == target.get_version()) &&
                        (OctetStr(((CTarget*)(&target))->get_readcommunity()) ==
                         ((UTarget*)tmptarget)->get_security_name())) {
                      target_matches = TRUE;
                      break;
                    }
                  }
              }
            }
          } // end if (add_equal)
        } // end if tmpaddr.valid()...
      }
    }
  }
  // else no targets means all targets

  // figure out how many trapids, handle empty case as all trapids
  if ((notify_ids) && (trapid_count = notify_ids->size())) {
    Oid tmpoid;
    has_trapid = TRUE;
    // loop through all trapids in the collection
    for (int y=0; y < trapid_count; y++)       // for all trapids
    {
      if (notify_ids->get_element(tmpoid, y))
	continue;
      if (trapid == tmpoid) {
	trapid_matches = TRUE;
	break;
      }
    }
  }
  // else no trapids means all traps

  // Make the callback if the trap passed the filters
  if ((has_target && !target_matches) || (has_trapid && !trapid_matches))
    return FALSE;
  return TRUE;
}


int CNotifyEvent::Callback(SnmpTarget &target, Pdu &pdu, SnmpSocket fd, int status)
{
  Oid trapid;
  pdu.get_notify_id(trapid);

  // Make the callback if the trap passed the filters
  if ((m_snmp) && (notify_filter(trapid, target)))
  {
    int reason;

    if (SNMP_CLASS_TL_FAILED == status)
      reason = SNMP_CLASS_TL_FAILED;
    else
      reason = SNMP_CLASS_NOTIFICATION;

    //------[ call into the callback function ]-------------------------
    if (m_snmp->get_notify_callback())
      (m_snmp->get_notify_callback())(
	  reason,
	  m_snmp,			// snmp++ session who owns the req
	  pdu,			// trap pdu
	  target,			// target
	  m_snmp->get_notify_callback_data()); // callback data
  }
  return SNMP_CLASS_SUCCESS;
}


//----[ CNotifyEventQueueElt class ]--------------------------------------

CNotifyEventQueue::CNotifyEventQueueElt::CNotifyEventQueueElt(
                                           CNotifyEvent *notifyevent,
					   CNotifyEventQueueElt *next,
					   CNotifyEventQueueElt *previous)
  : m_notifyevent(notifyevent), m_Next(next), m_previous(previous)
{
  /* Finish insertion into doubly linked list */
  if (m_Next)     m_Next->m_previous = this;
  if (m_previous) m_previous->m_Next = this;
}

CNotifyEventQueue::CNotifyEventQueueElt::~CNotifyEventQueueElt()
{
  /* Do deletion form doubly linked list */
  if (m_Next)        m_Next->m_previous = m_previous;
  if (m_previous)    m_previous->m_Next = m_Next;
  if (m_notifyevent) delete m_notifyevent;
}

CNotifyEvent *CNotifyEventQueue::CNotifyEventQueueElt::TestId(Snmp *snmp)
{
  if (m_notifyevent && (m_notifyevent->GetId() == snmp))
    return m_notifyevent;
  return 0;
}


//----[ CNotifyEventQueue class ]--------------------------------------
CNotifyEventQueue::CNotifyEventQueue(EventListHolder *holder, Snmp *session)
  : m_head(NULL,NULL,NULL), m_msgCount(0), m_notify_fds(0),
    m_notify_fd_count(0), m_listen_port(SNMP_TRAP_PORT),
    my_holder(holder), m_snmpSession(session)
{
//TM: could do the trap registration setup here but seems better to
//wait until the app actually requests trap receives by calling
//notify_register().
}

CNotifyEventQueue::~CNotifyEventQueue()
{
  CNotifyEventQueueElt *leftOver;

  /* walk the list deleting any elements still on the queue */
  lock();
  while ((leftOver = m_head.GetNext()))
    delete leftOver;
  unlock();
}

SnmpSocket CNotifyEventQueue::get_notify_fd(const UdpAddress match_addr) const
{
  SnmpSocket found_fd = INVALID_SOCKET;
  int max_bits_matched = 0;
  IpAddress ip_match = IpAddress(match_addr);

  for (int i = 0; i < m_notify_fd_count; i++)
  {
    IpAddress ip = m_notify_addrs[i];
    int bits = ip_match.get_match_bits(ip);

    debugprintf(5, "Compared %s to %s, bits %d", 
		ip.get_printable(), ip_match.get_printable(), bits);

    if (bits > max_bits_matched)
    {
	max_bits_matched = bits;
	found_fd = m_notify_fds[i];
    }
  }

  return found_fd;
}

SnmpSocket CNotifyEventQueue::get_notify_fd(const int i) const
{
  if ((i < 0) || ( i >= m_notify_fd_count))
      return INVALID_SOCKET;

  return m_notify_fds[i];
}

int CNotifyEventQueue::AddEntry(Snmp *snmp,
				const OidCollection &trapids,
				const TargetCollection &targets,
				const AddressCollection &addresses)
{
  if (snmp != m_snmpSession)
  {
    debugprintf(0, "WARNING: Adding notification event for other Snmp object");
  }

  lock();

  if (!m_msgCount)
  {
    //m_notify_addrs = addresses;
    if (m_notify_addrs.size() == 0)
    {
      UdpAddress tmp_addr = snmp->get_listen_address();
      tmp_addr.set_port(m_listen_port);
      m_notify_addrs += tmp_addr;
    }

    // allocate fd array
    m_notify_fds = new SnmpSocket[m_notify_addrs.size()];
    if (!m_notify_fds)
    {
      m_notify_addrs.clear();
      unlock();
      return SNMP_CLASS_RESOURCE_UNAVAIL;
    }
    for (int k=0; k < m_notify_addrs.size(); k++)
      m_notify_fds[k] = INVALID_SOCKET;
    m_notify_fd_count = m_notify_addrs.size();

    int status = SNMP_CLASS_SUCCESS;

    for (int i = 0; i < m_notify_fd_count ; i++)
    {
      // This is the first request to receive notifications
      // Set up the socket for the snmp trap port (162) or the
      // specified port through set_listen_port()
      UdpAddress tmp_udp_addr(m_notify_addrs[i]);
      bool is_v4_address = (tmp_udp_addr.get_ip_version() == Address::version_ipv4);

      if (is_v4_address)
      {
	struct sockaddr_in mgr_addr;

	// open a socket to be used for the session
	if ((m_notify_fds[i] = socket( AF_INET, SOCK_DGRAM,0)) < 0)
	{
#ifdef WIN32
	  int werr = WSAGetLastError();
	  if (EMFILE == werr ||WSAENOBUFS == werr || ENFILE == werr)
	      status = SNMP_CLASS_RESOURCE_UNAVAIL;
	  else if (WSAEHOSTDOWN == werr)
	      status = SNMP_CLASS_TL_FAILED;
	  else
	      status = SNMP_CLASS_TL_UNSUPPORTED;
#else
	  if (EMFILE == errno || ENOBUFS == errno || ENFILE == errno)
	      status = SNMP_CLASS_RESOURCE_UNAVAIL;
	  else if (EHOSTDOWN == errno)
	      status = SNMP_CLASS_TL_FAILED;
	  else
	      status = SNMP_CLASS_TL_UNSUPPORTED;
#endif
	  break;
	}

	// set up the manager socket attributes
	unsigned long inaddr = inet_addr(IpAddress(m_notify_addrs[i]).get_printable());
	memset(&mgr_addr, 0, sizeof(mgr_addr));
	mgr_addr.sin_family = AF_INET;
	mgr_addr.sin_addr.s_addr = inaddr; // was htonl( INADDR_ANY);
	mgr_addr.sin_port = htons(tmp_udp_addr.get_port());
#ifdef CYGPKG_NET_OPENBSD_STACK
	mgr_addr.sin_len = sizeof(mgr_addr);
#endif

	// bind the socket
	if (bind(m_notify_fds[i], (struct sockaddr *) &mgr_addr,
		 sizeof(mgr_addr)) < 0)
	{
#ifdef WIN32
	  int werr = WSAGetLastError();
	  if (WSAEADDRINUSE  == werr)
	    status = SNMP_CLASS_TL_IN_USE;
	  else if (WSAENOBUFS == werr)
	    status = SNMP_CLASS_RESOURCE_UNAVAIL;
	  else if (werr == WSAEAFNOSUPPORT)
	    status = SNMP_CLASS_TL_UNSUPPORTED;
	  else if (werr == WSAENETUNREACH)
	    status = SNMP_CLASS_TL_FAILED;
	  else if (werr == EACCES)
	    status = SNMP_CLASS_TL_ACCESS_DENIED;
	  else
	    status = SNMP_CLASS_INTERNAL_ERROR;
#else
	  if (EADDRINUSE  == errno)
	    status = SNMP_CLASS_TL_IN_USE;
	  else if (ENOBUFS == errno)
	    status = SNMP_CLASS_RESOURCE_UNAVAIL;
	  else if (errno == EAFNOSUPPORT)
	    status = SNMP_CLASS_TL_UNSUPPORTED;
	  else if (errno == ENETUNREACH)
	    status = SNMP_CLASS_TL_FAILED;
	  else if (errno == EACCES)
	    status = SNMP_CLASS_TL_ACCESS_DENIED;
	  else
	  {
	    debugprintf(0, "Uncatched errno value %d, returning internal error.",
			errno);
	    status = SNMP_CLASS_INTERNAL_ERROR;
	    break;
	  }
#endif
	  debugprintf(0, "Fatal: could not bind to %s",
		      m_notify_addrs[i].get_printable());

	  break;
	}
	debugprintf(3, "Bind to %s for notifications, fd %d.",
		    m_notify_addrs[i].get_printable(), m_notify_fds[i]);

      } // is_v4_address
      else
      {
	// not is_v4_address
#ifdef SNMP_PP_IPv6
	// open a socket to be used for the session
	if ((m_notify_fds[i] = socket(AF_INET6, SOCK_DGRAM,0)) < 0)
	{
#ifdef WIN32
	  int werr = WSAGetLastError();
	  if (EMFILE == werr ||WSAENOBUFS == werr || ENFILE == werr)
	      status = SNMP_CLASS_RESOURCE_UNAVAIL;
	  else if (WSAEHOSTDOWN == werr)
	      status = SNMP_CLASS_TL_FAILED;
	  else
	      status = SNMP_CLASS_TL_UNSUPPORTED;
#else
	  if (EMFILE == errno || ENOBUFS == errno || ENFILE == errno)
	      status = SNMP_CLASS_RESOURCE_UNAVAIL;
	  else if (EHOSTDOWN == errno)
	      status = SNMP_CLASS_TL_FAILED;
	  else
	      status = SNMP_CLASS_TL_UNSUPPORTED;
#endif
	  break;
	}

	// set up the manager socket attributes
	struct sockaddr_in6 mgr_addr;
	memset(&mgr_addr, 0, sizeof(mgr_addr));
	
	unsigned int scope = 0;

	OctetStr addrstr = ((IpAddress &)tmp_udp_addr).IpAddress::get_printable();

	if (tmp_udp_addr.has_ipv6_scope())
	{
	  scope = tmp_udp_addr.get_scope();

	  int y = addrstr.len() - 1;
	  while ((y>0) && (addrstr[y] != '%'))
	  {
	    addrstr.set_len(addrstr.len() - 1);
	    y--;
	  }
	  if (addrstr[y] == '%')
	    addrstr.set_len(addrstr.len() - 1);
	}

	if (inet_pton(AF_INET6, addrstr.get_printable(),
		      &mgr_addr.sin6_addr) < 0)
	{
	  LOG_BEGIN(ERROR_LOG | 1);
	  LOG("Notify transport: inet_pton returns (errno) (str)");
	  LOG(errno);
	  LOG(strerror(errno));
	  LOG_END;
	  status = SNMP_CLASS_INVALID_ADDRESS;
	  break;
	}

	mgr_addr.sin6_family = AF_INET6;
	mgr_addr.sin6_port = htons(tmp_udp_addr.get_port());
	mgr_addr.sin6_scope_id = scope;

	// bind the socket
	if (bind(m_notify_fds[i], (struct sockaddr *) &mgr_addr,
		 sizeof(mgr_addr)) < 0)
	{
#ifdef WIN32
	  int werr = WSAGetLastError();
	  if (WSAEADDRINUSE  == werr)
	    status = SNMP_CLASS_TL_IN_USE;
	  else if (WSAENOBUFS == werr)
	    status = SNMP_CLASS_RESOURCE_UNAVAIL;
	  else if (werr == WSAEAFNOSUPPORT)
	    status = SNMP_CLASS_TL_UNSUPPORTED;
	  else if (werr == WSAENETUNREACH)
	    status = SNMP_CLASS_TL_FAILED;
	  else if (werr == EACCES)
	    status = SNMP_CLASS_TL_ACCESS_DENIED;
	  else
	    status = SNMP_CLASS_INTERNAL_ERROR;
#else
	  if (EADDRINUSE  == errno)
	    status = SNMP_CLASS_TL_IN_USE;
	  else if (ENOBUFS == errno)
	    status = SNMP_CLASS_RESOURCE_UNAVAIL;
	  else if (errno == EAFNOSUPPORT)
	    status = SNMP_CLASS_TL_UNSUPPORTED;
	  else if (errno == ENETUNREACH)
	    status = SNMP_CLASS_TL_FAILED;
	  else if (errno == EACCES)
	    status = SNMP_CLASS_TL_ACCESS_DENIED;
	  else
	  {
	    debugprintf(0, "Uncatched errno value %d, returning internal error.",
			errno);
	    status = SNMP_CLASS_INTERNAL_ERROR;
	    break;
	  }
#endif
	  debugprintf(0, "Fatal: could not bind to %s",
		      m_notify_addrs[i].get_printable());

	  break;
	}
	debugprintf(3, "Bind to %s for notifications, fd %d.",
		    m_notify_addrs[i].get_printable(), m_notify_fds[i]);
#else
	debugprintf(0, "User error: Enable IPv6 and recompile snmp++.");
	status = SNMP_CLASS_TL_UNSUPPORTED;
	break;
#endif

      } // not is_v4_address
    } // for

    if (status != SNMP_CLASS_SUCCESS)
    {
      // Free all fds...
      for (int z=0; z < m_notify_addrs.size(); z++)
	if (m_notify_fds[z] != INVALID_SOCKET)
	  close(m_notify_fds[z]);
      delete [] m_notify_fds;
      m_notify_fds = 0;
      m_notify_fd_count = 0;

      unlock();

      return status;
    }
  }

  CNotifyEvent *newEvent = new CNotifyEvent(snmp, trapids, targets,
					    m_notify_addrs);

  /*---------------------------------------------------------*/
  /* Insert entry at head of list, done automagically by the */
  /* constructor function, so don't use the return value.    */
  /*---------------------------------------------------------*/
  (void) new CNotifyEventQueueElt(newEvent, m_head.GetNext(), &m_head);
  m_msgCount++;
  unlock();
  return SNMP_CLASS_SUCCESS;
}


CNotifyEvent *CNotifyEventQueue::GetEntry(Snmp * snmp) REENTRANT ({
  CNotifyEventQueueElt *msgEltPtr = m_head.GetNext();
  CNotifyEvent *returnVal = NULL;

  while (msgEltPtr){
    if ((returnVal = msgEltPtr->TestId(snmp)))
      return returnVal;
    msgEltPtr = msgEltPtr->GetNext();
  }
  return 0;
})

void CNotifyEventQueue::DeleteEntry(Snmp *snmp)
{
  lock();
  CNotifyEventQueueElt *msgEltPtr = m_head.GetNext();

  while (msgEltPtr){
    if (msgEltPtr->TestId(snmp)){
      delete msgEltPtr;
      m_msgCount--;
      break;
    }
    msgEltPtr = msgEltPtr->GetNext();
  }

  if (m_msgCount <= 0)
  {
    for(int i=0; i < m_notify_fd_count; i++)
    {
      // shut down the trap socket (if valid) if not using it.
      if (m_notify_fds[i] != (int)INVALID_SOCKET)
      {
	debugprintf(3, "Closing notifications port %s, fd %d.",
		    m_notify_addrs[i].get_printable(), m_notify_fds[i]);
	close(m_notify_fds[i]);
	m_notify_fds[i] = INVALID_SOCKET;
      }
    }
    if (m_notify_fds) delete [] m_notify_fds;
    m_notify_fds = 0;
    m_notify_fd_count = 0;
  }
  unlock();
}

#ifdef HAVE_POLL_SYSCALL
int CNotifyEventQueue::GetFdCount()
{
  SnmpSynchronize _synchronize(*this); // instead of REENTRANT()
  return m_notify_fd_count;
}

bool CNotifyEventQueue::GetFdArray(struct pollfd *readfds,
				   int &remaining)
{
  SnmpSynchronize _synchronize(*this); // instead of REENTRANT()

  if (m_notify_fd_count > 0) {
    for (int i = 0; i < m_notify_fd_count; i++)
    {
	if (remaining == 0)
	    return false;
	readfds[i].fd = m_notify_fds[i];
	readfds[i].events = POLLIN;
	remaining--;
    }
  }
  return true;
}

int CNotifyEventQueue::HandleEvents(const struct pollfd *readfds,
				    const int fds)
{
  SnmpSynchronize _synchronize(*this); // instead of REENTRANT()

  int status = SNMP_CLASS_SUCCESS;

  if (m_notify_fd_count == 0)
    return status;

  for (int i=0; i < fds; i++)
  {
    Pdu pdu;
    SnmpTarget *target = NULL;

    if ((readfds[i].revents & POLLIN) == 0)
	continue; // nothing to receive

    // find socket in local structure
    int nr = -1;
    if ((i < m_notify_fd_count) && (m_notify_fds[i] == readfds[i].fd))
      nr = i;
    else
    {
      for (int j=0; j < m_notify_fd_count; j++)
	if (readfds[i].fd == m_notify_fds[j])
	{
	  nr = j;
	  break;
	}

      if (nr == -1)
	continue; // no socket found
    }

    status = receive_snmp_notification(m_notify_fds[nr], *m_snmpSession,
				       pdu, &target);

    if (SNMP_CLASS_SUCCESS == status ||
	SNMP_CLASS_TL_FAILED == status)
    {
      // If we have transport layer failure, the app will want to
      // know about it.
      // Go through each snmp object and check the filters, making
      // callbacks as necessary
      if (!target) target = new SnmpTarget();

      CNotifyEventQueueElt *notifyEltPtr = m_head.GetNext();
      while (notifyEltPtr)
      {
	notifyEltPtr->GetNotifyEvent()->Callback(*target, pdu,
						 m_notify_fds[nr], status);
	notifyEltPtr = notifyEltPtr->GetNext();
      } // for each snmp object
    }
    if (target) // receive_snmp_notification calls new
      delete target;
  }

  return status;
}

#else

void CNotifyEventQueue::GetFdSets(int &maxfds,
				  fd_set &readfds,
				  fd_set &/*writefds*/,
				  fd_set &/*exceptfds*/) REENTRANT ({
  if (m_notify_fd_count > 0) {
    for (int i = 0; i < m_notify_fd_count; i++)
    {
      FD_SET(m_notify_fds[i], &readfds);
      if (maxfds < m_notify_fds[i] + 1)
	maxfds = SAFE_INT_CAST(m_notify_fds[i] + 1);
    }
  }
  return;
})

int CNotifyEventQueue::HandleEvents(const int /*maxfds*/,
				    const fd_set &readfds,
				    const fd_set &/*writefds*/,
				    const fd_set &/*exceptfds*/) REENTRANT ({
  int status = SNMP_CLASS_SUCCESS;

  if (m_notify_fd_count == 0)
    return status;

  for (int i=0; i < m_notify_fd_count; i++)
  {
    Pdu pdu;
    SnmpTarget *target = NULL;

    // pull the notifiaction off the socket
    if (FD_ISSET(m_notify_fds[i], (fd_set*)&readfds)) {
      status = receive_snmp_notification(m_notify_fds[i], *m_snmpSession,
					 pdu, &target);

      if (SNMP_CLASS_SUCCESS == status ||
	  SNMP_CLASS_TL_FAILED == status) {
	// If we have transport layer failure, the app will want to
	// know about it.
	// Go through each snmp object and check the filters, making
	// callbacks as necessary

        // LiorK: on failure target will be NULL
	if (!target)
	  target = new SnmpTarget();

	CNotifyEventQueueElt *notifyEltPtr = m_head.GetNext();
	while (notifyEltPtr){

	  notifyEltPtr->GetNotifyEvent()->Callback(*target, pdu,
						   m_notify_fds[i], status);
	  notifyEltPtr = notifyEltPtr->GetNext();
	} // for each snmp object
      }
      if (target) // receive_snmp_notification calls new
	delete target;
    }
  }
  return status;
})

#endif // HAVE_POLL_SYSCALL

#ifdef SNMP_PP_NAMESPACE
}; // end of namespace Snmp_pp
#endif 

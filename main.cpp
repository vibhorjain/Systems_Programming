#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>
#include <netinet/in.h>
#include "SocketUtilities.h"

#define MAXEVENTS 100

using namespace std;

int main (int argc, char *argv[])
{
  int sfd, s;
  int efd;
  struct epoll_event event;
  struct epoll_event *events;

  if (argc != 2)
    {
      cerr<< "Follow this input pattern: "<<argv[0]<<" [port]\n";
      exit (EXIT_FAILURE);
    }

  sfd = SocketUtilities::create_and_bind (argv[1]);
  if (sfd == -1)
    abort ();

	
  s = SocketUtilities::make_socket_non_blocking (sfd);
  if (s == -1)
    abort ();

  s = listen (sfd, SOMAXCONN);	//The maximum number of pending connection requests queued for any listening socket. The default is 10.
  if (s == -1)
    {
      perror ("listen");
      abort ();
    }

  efd = epoll_create (sizeof event);
  if (efd == -1)
    {
      perror ("epoll_create");
      abort ();
    }

  event.data.fd = sfd;
  event.events = EPOLLIN | EPOLLET;
  s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);
  
  if (s == -1)
    {
      perror ("epoll_ctl");
      abort ();
    }

  /* Buffer where events are returned */
  events = (epoll_event*)calloc (MAXEVENTS, sizeof event);

  /* The event loop */
  while (1)
    {
      int n, i;

      n = epoll_wait (efd, events, MAXEVENTS, -1);
	  
    for (i = 0; i < n; i++)
	{
	  if ((events[i].events & EPOLLERR) ||
              (events[i].events & EPOLLHUP) ||
              (!(events[i].events & EPOLLIN)))
	    {
          
		  /* An error has occurred on this fd, or the socket is not ready for reading --> wrong notification, raise error!! */
		  cerr<<"Error in epoll, wrong read notification!";
	      close (events[i].data.fd);
	      continue;
	    }

	  else if (sfd == events[i].data.fd)
	    {
              /* We have a notification on the listening socket, which means one or more incoming connections. */
              while (1)
                {
                  struct sockaddr in_addr;
                  socklen_t in_len;
                  int infd;
                  char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                  in_len = sizeof in_addr;
                  infd = accept (sfd, &in_addr, &in_len);

                  if (infd == -1)
                    {
                      if ((errno == EAGAIN) ||
                          (errno == EWOULDBLOCK))
                        {
                          /* We have processed all incoming connections. */
                          break;
                        }
                      else
                        {
                          perror ("accept");
                          break;
                        }
                    }

                  s = getnameinfo (&in_addr, in_len,
                                   hbuf, sizeof hbuf,
                                   sbuf, sizeof sbuf,
                                   NI_NUMERICHOST | NI_NUMERICSERV);
                  if (s == 0)
                    {
				  
						  cout<<"*****************************************************************"<<endl;
						  cout<<"FILE DESCRIPTOR ASSIGNED: "<<infd<<endl;
						  cout<<"CLIENT IP: "<<hbuf<<endl<<"CLIENT PORT: "<<sbuf<<endl;
						  cout<<"*****************************************************************"<<endl;

                    }

                  /* Make the incoming socket non-blocking and add it to the list of fds to monitor. */
                  s = SocketUtilities::make_socket_non_blocking (infd);
                  if (s == -1)
                    abort ();

                  event.data.fd = infd;
                  event.events = EPOLLIN | EPOLLET;
                  s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
                  if (s == -1)
                    {
                      perror ("epoll_ctl");
                      abort ();
                    }
                }
              continue;
            }
          else
            {
              /* Read and display. Epoll is in ET, no new event for pending data, so cross check using EAGAIN */
              int done = 0;

              while (1)
                {
                  ssize_t count;
                  char buf[512] = {0};

                  count = read (events[i].data.fd, buf, sizeof buf);
                  if (count == -1)
                    {
                      /* If errno == EAGAIN, that means we have read all data. So go back to the main loop. */
                      if (errno != EAGAIN)
                        {
                          perror ("read");
                          done = 1;
                        }
                      break;
                    }
                  else if (count == 0)
                    {
                      /* End of file. The remote has closed the connection. */
                      done = 1;
                      break;
                    }

				  cout<<buf<<endl;
				  
				  if (s == -1)
                    {
                      perror ("write");
                      abort ();
                    }
                }

              if (done)
                {
                  cout<<"Closed connection on descriptor: "<<events[i].data.fd<<endl;
                  close (events[i].data.fd);  //removing fd from epoll list
                }
            }
        }
    }

  free (events);

  close (sfd);

  return EXIT_SUCCESS;
}

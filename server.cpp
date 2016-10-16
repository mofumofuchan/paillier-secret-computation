/* Copyright (c) 2016 Kana Shimizu and Yusuke Okimoto */
#include "sumall.h"
#include "comm.h"
#include<sys/time.h>
#include <unistd.h>

//#define DEBUG

#ifdef DEBUG
#define DBG_PRT(...)  printf(__VA_ARGS__)
#else
#define DBG_PRT(...)
#endif

double
get_wall_time ()
{
    struct timeval time;
    if (gettimeofday (&time, NULL))
      {
	  return 0;
      }
    return (double) time.tv_sec + (double) time.tv_usec * .000001;
}

char *addr;
char *infile;
int aflg;
int port = 0;
int max_con;
int keyLen = 1024;
std::string tmp_dir_path;
std::string key_dir_path;
int
    compute_server_ID;

int
setParam (int argc, char **argv)
{
    int
	opt;
    aflg = 0;
    max_con = 1;
    int
	nopt = 0;
    tmp_dir_path = "";
    key_dir_path = "";
    while ((opt = getopt (argc, argv, "a:d:k:n:p:s:l:")) != -1)
      {
	  switch (opt)
	    {
	    case 'a':
		aflg = 1;
		addr = optarg;
		break;
	    case 'd':
		tmp_dir_path = optarg;
		break;
	    case 'k':
		key_dir_path = optarg;
		break;
	    case 'n':
		max_con = atoi (optarg);
		break;
	    case 'p':
		port = atoi (optarg);
		nopt++;
		break;
	    case 'l':
		compute_server_ID = atoi (optarg);
		nopt++;
		break;
	    case 's':
		keyLen = atoi (optarg);
		break;
	    default:
		fprintf (stderr,
			 "Usage: %s [-a address] [-d tmpfile_dir_path] [-k key_dir_path] [-n max_connections] -p port -l compute_server's ID \n",
			 argv[0]);
		exit (EXIT_FAILURE);
	    }
      }
    if (nopt == 2)
      {
	  return (0);
      }
    else
      {
	  fprintf (stderr,
		   "Usage: %s [-a address] [-d tmpfile_dir_path] [-k key_dir_path] [-n max_connections] -p port -l compute_server's ID \n",
		   argv[0]);
	  exit (1);
      }
}

int
main (int argc, char **argv)
{
    setParam (argc, argv);

    std::string publicKeyFile = key_dir_path + "pubkey";
    std::string privateKeyFile = key_dir_path + "prvkey";

    PublicKey pub;
    PrivateKey prv;

    double
	wts,
	wte;

    printf ("Generate keys.\n");
    prv.init (pub, keyLen);
    prv.save (privateKeyFile);
    pub.save (publicKeyFile);
    printf ("Write keys.\n");

    int
	sock0;
    if (!port)
	sock0 = prepSSock ();
    else
	sock0 = prepSSock (port);
    while (1)
      {
	  int
	      recvSize = 0;
	  int
	      result;

	  int
	      sock = acceptSSock (sock0);
	  wts = get_wall_time ();

	  sendFile (sock, (char *) publicKeyFile.c_str ());
	  std::cout << "sent a pubkey file\n";

	  std::string file = tmp_dir_path + "u2m_ran";
	  recvFile (sock, (char *) file.c_str ());
	  if ((result = decrypt (file, pub, prv)) == compute_server_ID)
	    {
		recvFile (sock, (char *) file.c_str ());
		  
	        long long result[VECTOR_DIMENSION];
		decrypt (file, pub, prv, result, VECTOR_DIMENSION);

		std::cout << "result from compute server: ";
		for (int i = 0; i < VECTOR_DIMENSION; i++)
		  {
		    std::cout << result[i] << ", ";
		  }
		std::cout << std::endl;
		
	    }
	  else
	    {
		std::cout << "ack from user: " << result << "\n";
	    }

	  closeSock (sock);
	  wte = get_wall_time ();
	  std::cerr << "server(Wall): " << wte - wts << "\n";
      }

    return (0);
}

/* Copyright (c) 2016 Kana Shimizu */
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
int max_con;
int keyLen = 1024;
std::string tmp_dir_path;
std::string key_dir_path;
std::string nodes_list;
std::string sumallfile;
char *
    host_m;			// master server
char *
    host_c;			// compute server
int
    port_m;
int
    port_c;
int
    ID;

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
    while ((opt = getopt (argc, argv, "m:c:p:q:l:a:d:k:n:i:s:o:")) != -1)
      {
	  switch (opt)
	    {
	    case 'm':
		nopt++;
		host_m = optarg;
		break;
	    case 'c':
		nopt++;
		host_c = optarg;
		break;
	    case 'p':
		nopt++;
		port_m = atoi (optarg);
		break;
	    case 'q':
		nopt++;
		port_c = atoi (optarg);
		break;
	    case 'l':
		nopt++;
		nodes_list = optarg;
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
	    case 'i':
		ID = atoi (optarg);
		nopt++;
		break;
	    case 's':
		keyLen = atoi (optarg);
		break;
	    case 'o':
		nopt++;
		sumallfile = optarg;
		break;
	    default:
		fprintf (stderr,
			 "Usage: %s [-d tmpfile_dir_path] [-k key_dir_path] [-n max_connections] -m master_server -p port_m -c compute_server -q port_c -i ID -l node_ID_list -o output\n",
			 argv[0]);
		exit (EXIT_FAILURE);
	    }
      }
    if (nopt == 6)
      {
	  return (0);
      }
    else
      {
	  fprintf (stderr,
		   "Usage: %s [-d tmpfile_dir_path] [-k key_dir_path] [-n max_connections] [-c compute_server] -m master_server -p port_m -q port_c -i ID -l node_ID_list -o output\n",
		   argv[0]);
	  exit (1);
      }
}

typedef struct pair_t
{
    int
	id;
    int
	chk;
} pair;

int
main (int argc, char **argv)
{
    setParam (argc, argv);

    std::string publicKeyFile = key_dir_path + "pubkey";
    PublicKey
	pub;

    double
	wts,
	wte;

    //read ID list
    std::vector < pair > nodes;
    std::ifstream ifs (nodes_list.c_str ());
    std::string str;

    std::vector < std::string > fileNames;

    while (getline (ifs, str))
      {
	  pair
	      tmpp;
	  tmpp.id = atoi (str.c_str ());
	  tmpp.chk = 0;
	  nodes.push_back (tmpp);
      }
    int
	nodes_n = nodes.size ();

    int
	sock0;
    if (!port_c)
	sock0 = prepSSock ();
    else
	sock0 = prepSSock (port_c);
    while (1)
      {
	  int
	      sock = acceptSSock (sock0);

	  //receive ID file
	  std::string file = tmp_dir_path + "id";
	  recvFile (sock, (char *) file.c_str ());
	  std::ifstream iifs (file.c_str ());
	  getline (iifs, str);
	  int
	      recID = atoi (str.c_str ());
	  std::cout << "recID: " << recID << "\n";

	  //receive encrypted value
	  file = tmp_dir_path + str + "_val";
	  for (int i = 0; i < nodes.size (); i++)
	    {
		if (nodes[i].id == recID)
		  {
		      if (!nodes[i].chk)
			{
			    nodes_n--;
			    fileNames.push_back (file);
			}
		      nodes[i].chk = 1;
		  }
	    }
	  recvFile (sock, (char *) file.c_str ());

	  closeSock (sock);
	  std::cerr << nodes_n << "\n";

	  if (nodes_n == 0)
	    {
		std::cout << "compute allsum and send it to master\n";

		int
		    sockm = prepCSock (host_m, port_m);

		std::string publicKeyFile = key_dir_path + "pubkey";
		recvFile (sockm, (char *) publicKeyFile.c_str ());	//receive pubkey from server

		pub.load (publicKeyFile);

		std::string file = tmp_dir_path + "c2m_ack";
		encrypt (file, pub, ID);
		sendFile (sockm, (char *) file.c_str ());

		wts = get_wall_time ();

		addall (fileNames, sumallfile, pub);	// compute sum

		wte = get_wall_time ();
		std::cerr << "Calc sum(Wall): " << wte - wts << "\n";

		sendFile (sockm, (char *) sumallfile.c_str ());

		closeSock (sockm);

		std::cout << "end of computation.\n";
		return (0);
	    }
      }

    return (0);
}

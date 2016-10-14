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

char *host_m;			// master server
char *host_c;			// compute server
int port_m;
int port_c;
std::string tmp_dir_path;
std::string key_dir_path;
#define VAL_DIMENSION 3 
long long
    val[VAL_DIMENSION];
int
    ID;

int
setParam (int argc, char **argv)
{
    int
	opt;
    int
	nopt = 0;
    extern int optind; 
    tmp_dir_path = "";
    key_dir_path = "";
    // while ((opt = getopt (argc, argv, "m:c:p:q:d:k:v:i:")) != -1)
    while ((opt = getopt (argc, argv, "m:c:p:q:d:k:i:")) != -1)
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

	    case 'd':
		tmp_dir_path = optarg;
		break;
	    case 'k':
		key_dir_path = optarg;
		break;
	    // case 'v':
	    // 	nopt++;
	    // 	val = atoll (optarg);
	    // 	break;
	    case 'i':
		nopt++;
		ID = atoi (optarg);
		break;

	    default:
		fprintf (stderr,
			 "Usage: %s [-d tmpfile_dir_path] [-k key_dir_path] -m master_server -c compute_server -p port_m -q port_c -i ID\n val1 val2 ...\n",
			 argv[0]);
		exit (EXIT_FAILURE);
	    }
      }
    if (nopt != 5)
      {
	fprintf (stderr,
		 "Usage: %s [-d tmpfile_dir_path] [-k key_dir_path]  -m master_server -c compute_server -p port_m -q port_c -i ID\n val1 val2 ...\n",
		   argv[0]);
	exit (1);
      }

    argc -= optind;
    argv += optind;

    if (argc != VAL_DIMENSION)
      {
	fprintf (stderr,
		 "Usage: %s [-d tmpfile_dir_path] [-k key_dir_path]  -m master_server -c compute_server -p port_m -q port_c -i ID\n val1 val2 ...",
		   argv[0]);
	exit (1);
      }
    
    for (int i=0; i<VAL_DIMENSION; i++)
      {
	printf("val[%d]:%s\n", i, argv[i]); // debug
	val[i] = atoll(argv[i]);
      }
    
    return (0);
    
}

int
main (int argc, char **argv)
{

    int
	recvSize = 0;
    int
	sentSize = 0;
    double
	wts,
	wte;

    PublicKey
	pub;

    wts = get_wall_time ();
    setParam (argc, argv);


    int
	sock = prepCSock (host_m, port_m);

    std::string publicKeyFile = key_dir_path + "pubkey";
    recvSize += recvFile (sock, (char *) publicKeyFile.c_str ());	//receive pubkey from server

    pub.load (publicKeyFile);

    std::string file = tmp_dir_path + "u2m_ack";
    encrypt (file, pub, ID);
    sentSize += sendFile (sock, (char *) file.c_str ());

    closeSock (sock);

    int
	sock2 = prepCSock (host_c, port_c);

    file = tmp_dir_path + "id";
    std::ofstream ofs ((char *) file.c_str ());
    ofs << ID << std::endl;

    sendFile (sock2, (char *) file.c_str ());	//send round and positions to server

    pub.load (publicKeyFile);
    file = tmp_dir_path + "c2s_ran";
    encrypt (file, pub, val, VAL_DIMENSION);
    sentSize += sendFile (sock, (char *) file.c_str ());

    closeSock (sock2);

    wte = get_wall_time ();
    std::cerr << "client(Wall): " << wte - wts << "\n";

    std::cerr.precision (3);
    std::cerr << "c2s, s2c(Mbyte): " << (double) sentSize / (1024 *
							     1024) << ", " <<
	(double) recvSize / (1024 * 1024) << "\n";

    return (0);
}

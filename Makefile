CXX=g++
CFLAGS_WARN=-Wall -Wextra -Wformat=2 -Wcast-qual -Wwrite-strings -Wfloat-equal -Wpointer-arith # -Wcast-align 
CFLAGS+=-O2 -I$(EXT_LIB_DIR)include $(CFLAGS_WARN)
LDFLAGS+=-L$(EXT_LIB_DIR)lib -lgmp

PAILLIER_LIB=libpaillier.a
CLIENT_OBJS=client.o comm.o
SERVER_OBJS=server.o comm.o
COMPUTE_OBJS=compute.o comm.o

CLIENT=client
SERVER=server
COMPUTE=compute

TARGET=$(CLIENT) $(SERVER) $(COMPUTE)
all: $(CLIENT) $(SERVER) $(COMPUTE)

$(CLIENT): $(CLIENT_OBJS)
	    $(CXX) $(CLIENT_OBJS) $(PAILLIER_LIB) $(LDFLAGS) -o $@
$(SERVER): $(SERVER_OBJS)
	    $(CXX) $(SERVER_OBJS) $(PAILLIER_LIB) $(LDFLAGS) -o $@
$(COMPUTE): $(COMPUTE_OBJS)
	    $(CXX) $(COMPUTE_OBJS) $(PAILLIER_LIB) $(LDFLAGS) -o $@


.cpp.o:
	$(CXX) $(CFLAGS) -c $<

$(CLIENT): sumall.h $(CLIENT_OBJS)
$(SERVER): sumall.h $(SERVER_OBJS)
$(COMPUTE): sumall.h $(COMPUTE_OBJS)

clean:
	rm -rf $(CLIENT) $(SERVER) $(COMPUTE) $(CLIENT_OBJS) $(SERVER_OBJS) $(COMPUTE_OBJS)

# DO NOT DELETE
client.o: client.cpp sumall.h paillier.h comm.h
server.o: server.cpp sumall.h paillier.h comm.h
compute.o: compute.cpp sumall.h paillier.h comm.h

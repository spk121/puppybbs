#pragma once
#include "puppy.h"
void openmsg();
void closemsg();
void topics(WORD t);
void listhdr(int n);
void listmsg(int n);
int ismsg(int n, int t);
WORD alltopics();
int recd(int n);
int oldest_msg();
int newest_msg();
struct _msg *getmsg(int n);
struct _msg *newmsg();
void savemsg(int lines);
void loadmsg(int n);


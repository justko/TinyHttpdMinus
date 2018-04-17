#pragma once
#include <stdio.h>
class MicroHttpd
{
public:
  MicroHttpd(unsigned short port = 0);
  void startup();
  void looping();
  void shutdown();
  static void acceptRequest(void *arg);

private:
  static void exeCGI(int client, const char *path, const char *method, const char *query_string);
  static int getLine(int sock, char *buff, int size);
  void errorDie(const char *sc);

  //
  static void headers(int client, const char *filename);
  static void cat(int client, FILE *resource);
  static void serveFile(int client, const char *filename);

  //special response
  //Inform the client that a request it has made has a problem.
  static void badRequest(int client);
  //Inform the client that a CGI script could not be executed.
  static void cannotExecute(int client);
  //Give a client a 404 not found status message.
  static void notFound(int client);
  //Inform the client that the requested web method has not been implemented.
  static void unimplemented(int client);

private:
  int httpd;
  unsigned short port;
  const static char *SERVER_STRING;
  const static unsigned STDIN;
  static const unsigned STDOUT;
  static const unsigned STDERR;
};
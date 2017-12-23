#ifndef PB_PROPERTIES_H
#define PB_PROPERTIES_H
#include <string>

// The static global properties read from the property tree file.  These should
// rarely change.
#define SS 80
#define TOPIC_COUNT 16
struct pup_static
{
  int nlimit;			/* max connection duration in minutes */
  int klimit;			/* max connection download in kilobytes */
  unsigned callsize;		/* max number of members stored */
  int messages;			/* max number of messages stored */
  int msgsize;			/* max size of any message */
  struct topic {
    std::string name;
    std::string desc;
  };
  std::vector<topic> topics;
  int maxbaud;			/* Max baud rate, lol. */
  std::string mdmstr;		/* Modem initialization string */
  std::string filepref;		/* File download area path */
  
  void load(const std::string &filename);
};

#endif

#include <pals.h>
#include <stdio.h>
#include <assert.h>

#define PERIOD 1000000000L   // 1.0 sec
#define SUPERVISOR	"supervisor"
#define SIDE1	"side1"
#define SIDE2	"side2"
#define CON_CMD	"con_cmd"
#define CON_STATE1	"con_state1"
#define CON_STATE2	"con_state2"

enum {
    NO_MSG = 0,
    TOGGLE = 1
};

extern struct pals_conf pals_conf;

#include "fsm.h"

/* poor substition for tests */

int
main(void)
{
    FSM fsm = { 0 };
    init_fsm(&fsm);
    uint8_t STATE_IDLE   = add_state(&fsm);
    uint8_t STATE_ACTIVE = add_state(&fsm);

    uint8_t INPUT_X      = add_input(&fsm);

    add_transition(&fsm, STATE_IDLE, STATE_ACTIVE, INPUT_X);
    add_transition(&fsm, STATE_ACTIVE, STATE_IDLE, INPUT_X);

    print_fsm(fsm);

    set_input_on(&fsm, INPUT_X);

    print_fsm(fsm);

    update_state(&fsm);
    print_fsm(fsm);

    update_state(&fsm);
    print_fsm(fsm);

    set_input_off(&fsm, INPUT_X);
    print_fsm(fsm);

    update_state(&fsm);
    print_fsm(fsm);

    return 0;

}

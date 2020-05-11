#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void
print_bits(uint16_t bits)
{
    unsigned int size = sizeof(uint16_t);
    unsigned int maxPow = 1 << (size*8-1);
    int i = 0;
    while (maxPow)
    {
        if (i % 4 == 0) printf(" ");
        printf("%u", !!(bits&maxPow ? 1 : 0));
        maxPow >>= 1;
        ++i;
    }
    printf("\n");
}

struct FSM
{
    uint16_t state;
    uint16_t *transitions;
    uint16_t *results;
    /* low 4 bits state_count, next 4 bits input_count, 8 high bits transition_count */
    uint16_t counts;
};

typedef struct FSM FSM;

uint8_t
init_fsm(FSM *fsm)
{
    fsm->transitions = (uint16_t*)malloc(sizeof(uint16_t));
    fsm->results = (uint16_t*)malloc(sizeof(uint16_t));
    return 0;
}

uint8_t
add_state(FSM *fsm)
{
    uint16_t state_count = fsm->counts & 0xf;
    uint16_t input_count = fsm->counts & 0xf0;
    uint16_t transition_count = fsm->counts & 0xff00;
    uint8_t index;

    index = state_count;
    state_count++;

    fsm->counts = transition_count | input_count | state_count;
    return index;
}

/* return a mask of the correct bit */
uint16_t
add_input(FSM *fsm)
{
    uint16_t state_count = fsm->counts & 0xf;
    uint16_t input_count = fsm->counts & 0xf0;
    uint16_t transition_count = fsm->counts & 0xff00;
    uint16_t mask;

    input_count = input_count >> 4;
    mask = (1 << (4 + input_count));
    input_count++;
    input_count = input_count << 4;

    fsm->counts = transition_count | input_count | state_count;
    return mask;
}

void
add_transition(FSM *fsm, uint8_t from_state, uint8_t to_state, uint16_t inputs)
{
    uint16_t row = 0;
    uint16_t state_count = fsm->counts & 0xf;
    uint16_t input_count = fsm->counts & 0xf0;
    uint16_t transition_count = fsm->counts & 0xff00;
    uint8_t index;
    long transitions_size;
    long results_size;
    uint16_t *transitions_buffer;
    uint16_t *results_buffer;
    uint16_t *result_line;
    uint16_t result_mask;
    uint16_t result_line_count;
    uint16_t result_line_index;
    uint16_t to_state_bits;

    transition_count = transition_count >> 8;
    index = transition_count;
    transition_count++;

    transitions_size = sizeof(uint16_t) * transition_count;

    printf("Allocating transitions to size: %ld bytes\n", transitions_size);
    transitions_buffer = (uint16_t*)realloc(fsm->transitions, transitions_size);
    if (transitions_buffer == NULL)
    {
        printf("Error reallocating transitions\n");
        exit(1);
    }  

    fsm->transitions = transitions_buffer;

    row |= (inputs);
    row |= from_state;
    
    fsm->transitions[index] = row;

    /* set result */
    result_line_count = ceil(transition_count/4.f);
    result_line_index = result_line_count - 1;
    results_size = sizeof(uint16_t) * ceil(transition_count/4.f);
    printf("results len: %d\n", (int)ceil(transition_count/4.f));

    printf("Allocating results to size: %ld bytes\n", results_size);
    results_buffer = (uint16_t*)realloc(fsm->results, results_size);
    if (results_buffer == NULL)
    {
        printf("Error reallocating results\n");
        exit(1);
    }  
    fsm->results = results_buffer;
    result_line = &fsm->results[result_line_index];

    result_mask = ~(0xf << 4 * (transition_count-1));
    *result_line &= result_mask;
    to_state_bits = to_state;
    to_state_bits <<= 4 * (transition_count-1);
    *result_line |= to_state_bits;

    transition_count = transition_count << 8;
    fsm->counts = transition_count | input_count | state_count;
}

void
update_state(FSM *fsm)
{
    int i;
    uint16_t transition_count = fsm->counts & 0xff00;
    transition_count >>= 8;
    for (i = 0; i < transition_count; ++i)
    {
        if (fsm->state == fsm->transitions[i])
        {
            uint8_t next_state = ((fsm->results[(int)(i/4.f)]) << 4*i) & 0xf;
            uint16_t inputs = fsm->state & 0xfff0;

            fsm->state = inputs | next_state;
            return;
        }
    }
}

void
set_input_on(FSM *fsm, uint16_t input)
{
    fsm->state |= input;
}

void
set_input_off(FSM *fsm, uint16_t input)
{
    fsm->state &= ~input;
}

void
print_fsm(FSM fsm)
{
    int i;
    int transition_count = (fsm.counts & 0xff00) >> 8;

    printf(" ===================\n");
    print_bits(fsm.state);
    printf(" ---- ---- ---- ----\n");

    for (i = 0; i < transition_count; ++i)
    {
        if (i > transition_count - 1)
            printf(" ---- ---- ---- ----\n");
        print_bits(fsm.transitions[i]);
    }
    printf(" ---- ---- ---- ----\n");

    for (i = 0; i < ceil(transition_count/4.f); ++i)
    {
        print_bits(fsm.results[i]);
    }
    printf(" ===================\n");
}


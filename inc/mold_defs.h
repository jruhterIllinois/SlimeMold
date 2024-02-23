/* 
 * File:   mold_defs.h
 * Author: jay
 *
 * Created on September 27, 2011, 8:40 PM
 */

#ifndef _MOLD_DEFS_H
#define	_MOLD_DEFS_H

#ifdef	__cplusplus
extern "C" {
#endif

    
//If going beyond 255 then GUI message has to change
#define SPATIAL_WIDTH  100
#define SPATIAL_HEIGHT 100

#define NUM_MAX_THRDS 100
#define PRCPTRN_ERR_THRSH   0.05
#define LRN_RATE_RGRSS      100

#define ACT_TYPE_THRSH      0
#define ACT_TYPE_SIGMOID    1
#define SIGMOID_VAR         10
#define ACT_TYPE_PCWISE     2
#define ACT_TYPE_GAUSS      3
 
    
#define NUM_HIDDEN_NODE     6
#define NUM_SYNAP_NET       3
#define NUM_NET_OUT         4

    

#define POPULATION_SZ  1000
#define GNRTN_SIZE POPULATION_SZ/2
#define SPECIES_GENES     CHROMO_LEN/2          //The species will share the first 10 genes
#define SUB_SPECIES_GENES 5            //The subspecies will share the next 5 genes
#define NUM_SPECIES       10
#define NUM_POP_SPECIES   POPULATION_SZ/NUM_SPECIES
    
    

/*
    Input *****
    //Energy Level                x1
    //Num of other organism       x2
    //Resource adaptablility      x3
    
    Output ****
    //Find another location       S1
    //Reproduce                   S2
    //Compete                     S3
    //Heal                        S4
    
    x1   x2   x3 | S1  S2  S3   S4
    -----------------------------------
    0    0     0 | 1   0   0    0
    0    0     1 | 0   0   0    1
    0    1     0 | 1   0   0    0
    0    1     1 | 0   0   0    1
    1    0     0 | 1   0   0    0
    1    0     1 | 0   1   0    1
    1    1     0 | 1   0   1    0
    1    1     1 | 0   0   1    0
    
    */


#define ENERGY_LVL_INPUT        0
#define NUM_LOC_ORG_INPUT       1
#define RESOURCE_ADAPT          2
    
#define FIND_ANOTHER_LOC_EVENT  0
#define REPRODUCE_EVENT         1
#define COMPETE_EVENT           2
#define HEAL_EVENT              3






#ifdef	__cplusplus
}
#endif

#endif	/* _MOLD_DEFS_H */


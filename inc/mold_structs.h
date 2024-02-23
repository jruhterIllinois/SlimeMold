/* 
 * File:   mold_structs.h
 * Author: jay
 *
 * Created on September 27, 2011, 8:39 PM
 */

#ifndef _MOLD_STRUCTS_H
#define	_MOLD_STRUCTS_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    
    
#define CHROMO_LEN 20
    
#define RSC_DRN_LVL_1  10
#define RSC_DRN_LVL_2  11
#define RSC_DRN_LVL_3  12

#define RSC_ADD_LVL_1  13
#define RSC_ADD_LVL_2  14
#define RSC_ADD_LVL_3  15
    
#define ORG_LE_CHROMO  8
#define ORG_LES_CHROMO 18
    
    /*
     In order to define the individual genetics the behavior between the organisms and the ecosystem is defined:
     The genetics can be seen as "life variables'
     ...[A][B][C][D][E] [F][G][H][I][J] [K][L][M][N][O] [P][Q][R][S][T]
     
     [A] - Location resource consumption parameter 1
     [B] - Location resource consumption parameter 2
     [C] - Location resource consumption parameter 3
     [D] - Location resource benefit parameter 1
     [E] - Location resource benefit parameter 2
     
     [F] - Location resource benefit parameter 3
     [G] - Organism's speed /Organism's energy consumption rate/metabolism
     [H] - The intelligence of the organism/ fast learning rate
     [I] - Organism's life expectancy
     [J] - Amount of Resources A parent give to child
     
     [K] - (Species gain) Location resource consumption parameter 1
     [L] - (Species gain) Location resource consumption parameter 2
     [M] - (Species gain) Location resource consumption parameter 3
     [N] - (Species gain) Location resource benefit parameter 1
     [O] - (Species gain) Location resource benefit parameter 2
     
     [P] - (Species gain) Location resource benefit parameter 3
     [Q] - (Species gain) Organism's speed /Organism's energy consumption rate/metabolism
     [R] - (Species gain) Organism's intelligent
     [S] - (Species gain) Organism's life expectancy
     [T] - (Species gain) Amount of Resources A parent give to child
     *
     */

/* An organism has various life forces/physiology
   1, Its energy storage
   2, Its size
   3, It's speed
   4, It's strength
 */

    typedef struct resource_lvl {
        
        double loc_drain;
        double loc_benefit;
        
    } RESOURCE_LVL;
    
//If an organism's energy falls to 0 the organism identity is reset to
    typedef struct organism_t {
        float         energy_lvl;
        int32_t         strength;
        uint64_t        identity;
        uint16_t        pop_idx;
        uint16_t        vert_loc;
        uint16_t        horz_loc;
        uint16_t        next_vert_loc;
        uint16_t        next_horz_loc;
        uint16_t     *  chromo;
        int64_t        birthdate;
        double           loc_net_rsc;
        uint64_t        num_org_coloc;
        RESOURCE_LVL    rsrc_lvl;
        uint8_t           spawn;
        uint8_t          combat;
        
    } ORGANISM_t;
    
    
    
    typedef struct disease_t {
        
        
        
    } DISEASE_t;
    
    
    
    typedef struct gui_msg {
        uint8_t horz_loc;
        uint8_t vert_loc;
    } GUI_MSG_t;
    
    

//The mold space is the structure that

typedef struct mold_space_t {
    /**************FIRST DIMENSIONS HEIGHT****************/

    //contains the value of all the resources
    //at a particular pnt on the map
    uint16_t rsrc_map[CHROMO_LEN];
    uint16_t num_org;
    
    uint16_t toxicity;// the toxicity is a permanent expenditure of energy
    
    uint8_t pxl_color_R; //The Red color is indicative of the number of toxicity at a location
    uint8_t pxl_color_G; //The Green color is indicative of the number of organisms at a location
    uint8_t pxl_color_B; //The Blue color is indicative of the number of resources at a location
    
} MOLD_SPACE_t;

    
    
    
typedef struct mold_time_t {

    uint64_t strt_cntr;
    

} MOLD_TIME_t;




typedef struct mcp_mdl_t {

    int *num_sigs;
    int num_train_atmpts;
    float learn_rate;
    float ** sigs;
    float * weights;
    float soma;
    char act_type;

} MCP_MDL_t;

typedef struct grp_mcp_t {

    int num_perceptron;
    MCP_MDL_t *  perceptron_grp ;

} GRP_MCP_t;

int combOrgEntry(int num_organism, uint32_t * pop_idx, MOLD_SPACE_t ** m_space_loc,
                     ORGANISM_t * population_loc,
                 uint32_t * pop_idx_loc);
    
void * ANNet_Organism_trn(ORGANISM_t * orgnsm_x, void * in, void * out);
void * ANNet_Organism_cmpt(ORGANISM_t * orgnsm_x, void * in, void * out);
float gamma_function_slime(float org_life, float alpha, float beta);
    

#ifdef	__cplusplus
}
#endif

#endif	/* _MOLD_STRUCTS_H */


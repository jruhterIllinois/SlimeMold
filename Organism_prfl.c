//
//  Organism_prfl.c
//  Slime mold v2
//
//  Created by Jeremy Ruhter on 1/11/20.
//  Copyright © 2020 Jeremy Ruhter. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>             // UNIX protocol
#include <netdb.h>

#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

#include "./inc/mold_bin.h"
#include "./inc/mold_defs.h"
#include "./inc/mold_structs.h"


//extern MOLD_SPACE_t ** m_space;
//extern ORGANISM_t * population;
//extern uint32_t * pop_idx;







//A 3x4 truth table is created to handle the decision processiong
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


extern uint64_t current_sim_time;
int organism_Loc_Combo[POPULATION_SZ] = {0};
int opt_org_Loc_Combo[POPULATION_SZ] = {0};
int opt_org_num = 0;
int locIndex = 0;



float slime_fact(uint8_t  in)
{
    if( in >1 )
        return( slime_fact( in - 1 )*(float)in);
    else
        return(in);
}


float gamma_function_slime(float org_life, float alpha, float beta)
{
    float func_out;

 
    beta = beta/330 + 0.3;
    alpha = alpha/6553.6 + 5;
    func_out = powf(beta,alpha);
    func_out = func_out*pow(org_life,alpha -1);
    func_out = func_out*exp(-beta*org_life);
    func_out = func_out/slime_fact((uint8_t)(alpha -1));

    
    return(func_out);
    
}



void * OrganismPrfile(void * arg1, void * args2)
{
    float nnet_input[NUM_SYNAP_NET];
    float nnet_output[NUM_NET_OUT];
    ORGANISM_t * org_input = arg1;
    uint16_t next_horz;
    uint16_t next_vert;
    
     /*
    //Compute the energy level of the organism
    computeOrganismEnergy(org_input, NULL);
      */
    
    //These need to be normalized
    computeOrganismEnergy(org_input,NULL);
    nnet_input[ENERGY_LVL_INPUT] = org_input->energy_lvl;
    nnet_input[NUM_LOC_ORG_INPUT] = org_input->num_org_coloc;
    nnet_input[RESOURCE_ADAPT] = org_input->loc_net_rsc;
    
    //based on the input variables compute the output of the neural net
    

    ANNet_Organism_cmpt(org_input, nnet_input, nnet_output);

    //IF the organism decides it needs to move the move
    if(nnet_output[FIND_ANOTHER_LOC_EVENT] > 0.5)
    {
        find_new_org_loc(org_input->horz_loc, org_input->vert_loc, &next_horz, &next_vert);
        org_input->next_horz_loc = next_horz;
        org_input->next_vert_loc = next_vert;
    }
    else if(nnet_output[HEAL_EVENT] > 0.5)
    {
        org_input->birthdate = org_input->birthdate +((float)rand()/(float)RAND_MAX)*5;
    }
    
    if(nnet_output[REPRODUCE_EVENT] > 0.5)
        org_input->spawn = 1.0;
    else
        org_input->spawn = 0.0;
    
    if(nnet_output[COMPETE_EVENT] > 0.5)
        org_input->combat = 1.0;
    else
       org_input->combat = 0.0;
    
    
    return(0);
}


void populateOrgComb(int data)
{
    organism_Loc_Combo[locIndex] = data;
    locIndex++;
}

float glb_res_score;

void computeOrganismBalance(MOLD_SPACE_t ** m_space_loc, ORGANISM_t * population_loc, uint32_t * pop_idx_loc)
{
    float res = 0.0;
    int i;
    int loc = 0;
    
    //
  
//    printf("The organisms at this location are - ");
    for(i = 0; i<locIndex; i++ )
    {
        loc = organism_Loc_Combo[i];
  //     printf("{ %i , %f, %f}", organism_Loc_Combo[i],  population_loc[loc].rsrc_lvl.loc_benefit, population_loc[loc].rsrc_lvl.loc_drain);
        res += population_loc[loc].loc_net_rsc;
    }
    
   // printf("\n");
   
    
    if(res > glb_res_score)
    {
        memcpy(opt_org_Loc_Combo,organism_Loc_Combo, POPULATION_SZ*sizeof(int));
        glb_res_score = res;
        opt_org_num = locIndex;
 //       printf("Setting Max \r\n");
    }
    
    bzero(organism_Loc_Combo, POPULATION_SZ*sizeof(int));
    locIndex = 0;
}

void combOrg(int arr[], int data[], int start, int end,
             int index, int r, MOLD_SPACE_t ** m_space_loc, ORGANISM_t * population_loc, uint32_t * pop_idx_loc)
{
    // Current combination is ready to be printed, print it
    if (index == r)
    {
        for (int j=0; j<r; j++)
            populateOrgComb( data[j] );
        computeOrganismBalance( m_space_loc, population_loc, pop_idx_loc);
        return;
    }
    
    for (int i=start; i<=end && end-i+1 >= r-index; i++)
    {
        data[index] = arr[i];
        combOrg(arr, data, i+1, end, index+1, r, m_space_loc, population_loc, pop_idx_loc);
    }
}



void find_new_org_loc(uint16_t  v, uint16_t  h, uint16_t * vs, uint16_t * hs)
{
    int32_t rand_inc =  rand() - (RAND_MAX/2);
    int32_t rand_inc2 =  rand() - (RAND_MAX/2);
    
    
    if(rand_inc> 0 )
        rand_inc = 1;
    else
        rand_inc = -1;
    
    if(rand_inc2> 0 )
        rand_inc2 = 1;
    else
        rand_inc2 = -1;
    
    
    if(v == 0)
    {
        vs[0] = v +1;
        if(h > 0)
            if(h < SPATIAL_WIDTH-2)
                hs[0] = h + rand_inc;
            else
                hs[0] = h-1;
        else
            hs[0] = h +1;
            
        
        
    }
    else if(v < (SPATIAL_HEIGHT - 2))
    {
        vs[0] = v + rand_inc2;
        if(h > 0)
            if(h < SPATIAL_WIDTH-2)
                hs[0] = h  + rand_inc;
            else
                hs[0] = h-1;
            else
                hs[0] = h +1;
        
    }
    else
    {
        vs[0] = v - 1;
        if(h > 0)
            if(h < SPATIAL_WIDTH-2)
                hs[0] = h  + rand_inc;
            else
                hs[0] = h -1;
            else
                hs[0] = h +1;
        
    }

    
}



int combOrgEntry(int num_organism, uint32_t * pop_idx, MOLD_SPACE_t ** m_space_loc,
                 ORGANISM_t * population_loc,
                 uint32_t * pop_idx_loc)
{
    int i = 0, j = 0;
    int data[100] = {0};
    uint16_t new_horz,old_horz;
    uint16_t new_vert, old_vert;
    
    if(num_organism >100)
    {
        printf("Too many organism \r\n");
        return(-1);
    }
    
    if(num_organism < 2)
    {
        //       printf("No organism combination nets a positive %i ", num_organism);
        for(i = 0; i< num_organism; i++ )
        {
            find_new_org_loc(population_loc[pop_idx_loc[i]].horz_loc, population_loc[pop_idx_loc[i]].vert_loc, &new_horz, &new_vert);
            
            population_loc[pop_idx_loc[i]].next_horz_loc = new_horz;
            population_loc[pop_idx_loc[i]].next_vert_loc = new_vert;
            population_loc[pop_idx_loc[i]].energy_lvl = population_loc[pop_idx_loc[i]].energy_lvl - 0.1;
            //  printf("Setting new location for %i %i %i\n",pop_idx_loc[i],new_horz, new_vert);
            
        }
        return(0);
        
    }
    
    //Reset the stats
    glb_res_score = -1e12;
    opt_org_num = 0;
    bzero(opt_org_Loc_Combo, POPULATION_SZ*sizeof(int));
    
    for(i = 1; i < num_organism+1;i++)
        combOrg(pop_idx, data, 0, num_organism-1, 0, i, m_space_loc, population_loc, pop_idx_loc);
    
    
    

    

    
    if(glb_res_score > 0.0)
    {
    
        //These get to stay
       // opt_org_Loc_Combo
      //  opt_org_num
    //    printf("NUM ORG %i \r\n", num_organism);
        for(i = 0; i< num_organism; i++ )
        {

            old_horz =  population_loc[pop_idx_loc[i]].next_horz_loc;
            old_vert = population_loc[pop_idx_loc[i]].next_vert_loc;
            
            find_new_org_loc(population_loc[pop_idx_loc[i]].horz_loc, population_loc[pop_idx_loc[i]].vert_loc, &new_horz, &new_vert);
            
            population_loc[pop_idx_loc[i]].next_horz_loc = new_horz;
            population_loc[pop_idx_loc[i]].next_vert_loc = new_vert;
            population_loc[pop_idx_loc[i]].energy_lvl = population_loc[pop_idx_loc[i]].energy_lvl - 0.1;
            
            //search the opt index to see if the organism needs to move
            for(j = 0; j< opt_org_num; j++)
            {
                if(pop_idx_loc[i] == opt_org_Loc_Combo[j])
                {
                    population_loc[pop_idx_loc[i]].next_horz_loc = old_horz;
                    population_loc[pop_idx_loc[i]].next_vert_loc = old_vert;
                    
                    population_loc[pop_idx_loc[i]].energy_lvl +=  (float)((float)population_loc[pop_idx_loc[i]].birthdate - (float)current_sim_time)/100;
                }
            }
            
            
            

        }
        
    }
    else
    {
        
        for(i = 0; i< num_organism; i++ )
        {
            
            find_new_org_loc(population_loc[pop_idx_loc[i]].horz_loc, population_loc[pop_idx_loc[i]].vert_loc, &new_horz, &new_vert);
            population_loc[pop_idx_loc[i]].next_horz_loc = new_horz;
            population_loc[pop_idx_loc[i]].next_vert_loc = new_vert;
            population_loc[pop_idx_loc[i]].energy_lvl = population_loc[pop_idx_loc[i]].energy_lvl - 1.0;
      
        }
    }
    
    return(0);
}


void * computeOrganismEnergy(void * arg1, void * args2)
{
    ORGANISM_t * organism_prfile = arg1;
    float out;
   // float alpha = (float)organism_prfile->chromo[ORG_LE_CHROMO];
  //  float beta  = (float)organism_prfile->chromo[ORG_LES_CHROMO];
    float alpha = 7;
    float beta  = 0.4;
    float life_in =(float) current_sim_time - (float)organism_prfile->birthdate;
    //the organism energy levels are computed as a curve function of age
    //if an organism chooses to heal then the energy levels
    //the life span follows a gamma distribution function with the function parameters being part of the genetic code
    
    out  = gamma_function_slime(life_in, alpha, beta);
    
    organism_prfile->energy_lvl = out;
    
    return(0);
}

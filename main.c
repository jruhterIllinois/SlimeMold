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


//
//  main.c
//  Slime mold v2
//
//  Created by Jeremy Ruhter on 12/27/19.
//  Copyright © 2019 Jeremy Ruhter. All rights reserved.
//


uint64_t comb_comp(uint16_t n, uint16_t r)
{
    uint16_t i;
    uint64_t num = 1;
    uint64_t den = 1;
    
    if(r >= n)
        return(1);
    
    for(i = n; i>0; i--)
        num = num * i;
    
    for(i=r; i>0; i--)
        den = den * i;
    
    for(i = n - r; i>0; i--)
        den = den *i;
    
    return((uint64_t)num/den);
}

uint64_t comb_comp_sum(uint16_t n)
{
    uint16_t idx;
    uint64_t sum_out = 1;
    
    for(idx =1; idx < n; idx++)
        sum_out = comb_comp(n, idx) + sum_out;
    
    return(sum_out);
    
}


MOLD_SPACE_t ** m_space;
ORGANISM_t * population;
uint32_t * pop_idx;







int count = 0;
/* arr[]  ---> Input Array
 data[] ---> Temporary array to store current combination
 start & end ---> Staring and Ending indexes in arr[]
 index  ---> Current index in data[]
 r ---> Size of a combination to be printed */
void combinationUtil(int arr[], int data[], int start, int end,
                     int index, int r)
{
    count++;
    // Current combination is ready to be printed, print it
    if (index == r)
    {
        for (int j=0; j<r; j++)
            printf("%d ", data[j]);
        printf("\n");
        return;
    }
    
    // replace index with all possible elements. The condition
    // "end-i+1 >= r-index" makes sure that including one element
    // at index will make a combination with remaining elements
    // at remaining positions
    for (int i=start; i<=end && end-i+1 >= r-index; i++)
    {
        data[index] = arr[i];
        combinationUtil(arr, data, i+1, end, index+1, r);
    }
}

uint64_t current_sim_time = 0;

void main(int argc, const char * argv[])
{
    

    uint16_t ** test_chromosomes;
    int16_t i = 0;
    int16_t j = 0;
    int16_t vert_loc = 0;
    int16_t horz_loc = 0;
    uint64_t gen_idx = 0;
    uint16_t org_idx = 0;
    uint16_t org_cmp_idx = 0;

    int orig_sock, // Original socket in server
    new_sock; // New socket from connect
    socklen_t clnt_len; // Length of client address
    struct sockaddr_in // Internet addr client & server
    clnt_adr, serv_adr;
    int len; // Misc counters, etc.
    int oldtype;
    char buf[255];
    uint32_t it_address;
    int set_value_out = 0;
    float learning_rate;
    
    //signal(SIGINT, sig_handler);



    uint16_t ** speces_genetic;
    
    /*
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

    float x1_t[8] = { 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0};
    float x2_t[8] = { 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0};
    float x3_t[8] = { 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0};
    
    float y1_t[8] = { 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0};
    float y2_t[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0};
    float y3_t[8] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0};
    float y4_t[8] = { 0.0, 1.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0};
    
    float train_data_in[3];
    float train_data_out[4];
    
    uint16_t    species_pop_count =0;
    uint16_t    species_index=0;
    uint32_t    org_pop_idx = 0;
    int arr[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
    int data[20] ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    GUI_MSG_t * msg_out;
    
    float loc_cum_res = 0.0;

    
    srand(time(NULL));
  
    
    /*
    float test1 = 20.0;
    float test2 = 0.0;
    float test3 = 40000.0;
    float test4 = 50.0;
    gamma_function_slime(&test1, &test2, &test3, &test4 );
    exit(0);
     */
    
    
//    int n = 20;
    //combinationUtil(arr, data, 0, n-1, 0, r);
  //  for(i = 1; i < n+1;i++)
   //     combinationUtil(arr, data, 0, n-1, 0, i);
    
    //combOrgEntry(5, arr);
 //   printf("Answer - %llu \n", comb_comp_sum(12));
   //  float decision_input[3] = {0.5, 0.0, 1.0};
   //  ANNet((void * )decision_input);
   //  exit(0);

    
    //the species genetic contains the initial defining genetic information for the species
    if((pop_idx = (uint32_t *)valloc(POPULATION_SZ*sizeof(uint32_t))) < 0)
    {
        perror("VALLOC FAIL");
        terminate_all(__LINE__,__FILE__);
    }
    bzero(pop_idx, POPULATION_SZ*sizeof(uint32_t));
    
    //the species genetic contains the initial defining genetic information for the species
    if((speces_genetic = (uint16_t **)valloc(NUM_SPECIES*sizeof(uint16_t *))) < 0)
    {
        perror("VALLOC FAIL");
        terminate_all(__LINE__,__FILE__);
    }
    bzero(speces_genetic, NUM_SPECIES*sizeof(uint16_t *));
    
    
    for(i = 0; i <NUM_SPECIES; i++)
    {
        
        if((speces_genetic[i] = (uint16_t *)valloc(SPECIES_GENES*sizeof(uint16_t))) < 0)
        {
            perror("VALLOC FAIL");
            terminate_all(__LINE__,__FILE__);
        }
        
        
        for(j = 0; j< SPECIES_GENES; j++)
            speces_genetic[i][j] = (uint16_t) rand();
        
    }
    
    ANNet_initialize(NULL);
    

    
    //DONT KNOW WHY ITS 10*
    if((population = ( ORGANISM_t * ) valloc( POPULATION_SZ *10*sizeof( ORGANISM_t *))) < 0)
    {
        perror("VALLOC FAIL");
        terminate_all(__LINE__,__FILE__);
    }
    bzero( population, POPULATION_SZ*10*sizeof( ORGANISM_t *));
    

    if((m_space = ( MOLD_SPACE_t **)valloc(SPATIAL_HEIGHT*sizeof( MOLD_SPACE_t *))) < 0)
    {
        perror("VALLOC FAIL");
        terminate_all(__LINE__,__FILE__);
    }
    bzero(m_space, SPATIAL_HEIGHT*sizeof( MOLD_SPACE_t *));
    

    for(i = 0; i <SPATIAL_HEIGHT; i++)
    {
        
        if((m_space[i] = (MOLD_SPACE_t *)valloc(SPATIAL_WIDTH*sizeof(MOLD_SPACE_t))) < 0)
        {
            perror("VALLOC FAIL");
            terminate_all(__LINE__,__FILE__);
        }
        bzero(m_space[i], SPATIAL_WIDTH*sizeof(MOLD_SPACE_t));
        
    }
    

    for(vert_loc = 0; vert_loc < SPATIAL_HEIGHT; vert_loc++)
    {
        for(horz_loc = 0; horz_loc < SPATIAL_WIDTH; horz_loc++)
        {
            
  /*          if((m_space[vert_loc][horz_loc].rsrc_map =  (uint16_t*)valloc(CHROMO_LEN*sizeof(uint16_t))) < 0)
            {
                perror("VALLOC FAIL");
                terminate_all(__LINE__,__FILE__);
            } */
            
            for(i = 0; i< CHROMO_LEN; i++)
                m_space[vert_loc][horz_loc].rsrc_map[i] =  (uint16_t)(((float)rand()/(float)RAND_MAX) *100);
        }
    }
    
    

    //Generate the chromosome space
    for( i = 0 ; i< POPULATION_SZ; i++)
    {
        
        
        if((population[i].chromo = (uint16_t *)valloc(CHROMO_LEN *sizeof(uint16_t))) < 0)
        {
            perror("VALLOC FAIL");
            terminate_all(__LINE__,__FILE__);
        }
        bzero(population[i].chromo , CHROMO_LEN*sizeof(uint16_t));
        
#ifdef PRNT_GNTC_CODE
        printf("Chromosome %i location %i has value ",  i,  j);
#endif
        
        for(j = 0; j< CHROMO_LEN; j++)
        {
            if(j < SPECIES_GENES)
                  population[i].chromo[j] = speces_genetic[species_index][j];
            else
                  population[i].chromo[j] = ((float) ((float)rand()/(float)RAND_MAX) *100.0);
#ifdef PRNT_GNTC_CODE
            printf(" %i",population[i].chromo[j]);
#endif
        }
        
        species_pop_count++;
#ifdef PRNT_GNTC_CODE
        printf("\n");
#endif
        
        //increment the species counter
        if(species_pop_count == (uint16_t)NUM_POP_SPECIES)
        {
            species_pop_count = 0;
            species_index++;
        }

    }

    
    //randomly scatter the population
    for(i = 0; i< POPULATION_SZ; i++)
    {
        population[i].horz_loc = (uint16_t)(((float)rand()/(float)RAND_MAX) *SPATIAL_WIDTH);
        population[i].vert_loc = (uint16_t)(((float)rand()/(float)RAND_MAX) *SPATIAL_HEIGHT);
        population[i].energy_lvl = 50.0;
        population[i].birthdate = 0;
        population[i].pop_idx = i;
        population[i].next_horz_loc = population[i].horz_loc;
        population[i].next_vert_loc = population[i].vert_loc;
        
    //    printf("Organism at %u %u \n",population[i].horz_loc , population[i].vert_loc);
    }
    
    
    printf("Training the initial population \r\n");
    for(org_pop_idx = 0; org_pop_idx <POPULATION_SZ; org_pop_idx++)
    {
        learning_rate = (float)population[org_pop_idx].chromo[7]/(float)65536;
        learning_rate *= 10000;
     //   for (i = 0; i<learning_rate; i++)
        for(i = 0; i<100; i++)
            for(j = 0; j<8; j++)
            {
                
                train_data_in[0] = x1_t[j];
                train_data_in[1] = x2_t[j];
                train_data_in[2] = x3_t[j];
                
                train_data_out[0] = y1_t[j];
                train_data_out[1] = y2_t[j];
                train_data_out[2] = y3_t[j];
                train_data_out[3] = y4_t[j];
                
                
                ANNet_Organism_trn(&population[org_pop_idx], train_data_in, train_data_out);
            }
    }
    printf("Population trained \r\n");
  //  ANNet(train_data_in);
 //   exit(0);
    
    
    if ((orig_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("generate error");
        terminate_all(__LINE__,__FILE__);
    }

    
    memset(&serv_adr, 0, sizeof (serv_adr)); // Clear structure
    serv_adr.sin_family = AF_INET; // Set address type
    serv_adr.sin_addr.s_addr = it_address;
    serv_adr.sin_port = htons( (uint16_t)9546  ); // Use our fake port
    
    
    it_address = htonl(INADDR_ANY); // Any interface
//    printf("Starting gui \r\n");
 //   execvp("python Slime_mold_guiu.py", NULL);
    
    // BIND
    if (bind(orig_sock, (struct sockaddr *) & serv_adr, sizeof (serv_adr)) < 0)
    {
        perror("bind error");
        close(orig_sock);
        terminate_all(__LINE__,__FILE__);
    }
    if (listen(orig_sock, 5) < 0)
    { // LISTEN
        perror("listen error");
        close(orig_sock);
        terminate_all(__LINE__,__FILE__);
    }

    clnt_len = sizeof (clnt_adr); // ACCEPT a connect
    if ((new_sock = accept(orig_sock,
                           (struct sockaddr *) & clnt_adr,
                           &clnt_len))     < 0)
    {
        perror("accept error");
        close(orig_sock);
        terminate_all(__LINE__,__FILE__);
    }
    
    printf("New socket formed \n");

    sprintf(buf,"Hello from the server \r\n");
   
    if((msg_out = (GUI_MSG_t *)valloc(POPULATION_SZ*sizeof(GUI_MSG_t))) < 0)
    {
        perror("VALLOC FAIL");
        terminate_all(__LINE__,__FILE__);
    }
    bzero(msg_out, POPULATION_SZ*sizeof(GUI_MSG_t));

    
    //exit(0);
    
/* THE genetic algorithm is an optimization technique for global search functions, though it is mimicked after
   the biological evolutionary process which is a protein/environment optimization problem.
 
   In this program we will model fitness as survivability of a simulated organism in a modeled ecosystem, the organism and ecosystem
   can be advanced to apply to any function optimization search, different organism will be developed to model emergence and organelle development */
    

    //THe execution of the simulation is performed by iterating over the entire map and at each location update the status of the organisms
    //per location
    printf("Starting Execution \r\n");

    while(1)
    {
        
        if(write(new_sock, msg_out , POPULATION_SZ*sizeof(GUI_MSG_t)) < 0)
        {
            close(orig_sock);
            terminate_all(__LINE__,__FILE__);
        }
        printf("Sent the data over the socket \r\n");
        usleep(500000);
        
        
        
        //First compute the map wide operations
        for(vert_loc = 0; vert_loc < SPATIAL_HEIGHT; vert_loc++)
        {
            for(horz_loc = 1; horz_loc < SPATIAL_WIDTH; horz_loc++)
              {
                 // printf("Executing at loc %i %i \r\n", vert_loc, horz_loc);
                  //   printf("Organism0 at %u %u \n",population[128].horz_loc , population[128].vert_loc);
                  //create a link list of the organism at the space
                  bzero(pop_idx, POPULATION_SZ*sizeof(uint32_t));
                  m_space[vert_loc][horz_loc].num_org = 0;
                  
                  //    printf("Organism1 at %u %u \n",population[128].horz_loc , population[128].vert_loc);
                  
                  for(org_idx = 0; org_idx < POPULATION_SZ; org_idx++)
                  {
                      if((population[org_idx].horz_loc == horz_loc)&&(population[org_idx].vert_loc == vert_loc))
                      {
                          pop_idx[m_space[vert_loc][horz_loc].num_org] = org_idx;
                          m_space[vert_loc][horz_loc].num_org++;
                          
                      }
                      
                  }
                  
                  //m_space[vert_loc][horz_loc]
 
                  /* The key in demonstrating emergence of organelles is creating cooperative symbiosis and a complete emergent organism would be all organism either dying off or creating a symbiosis, symbiosis will keep occuring until there are no competitions */
                  
                  /* Each of the organisms genetic components have to provide some environmental advantage
                     To create a symbiosis the organism will influence the environmental variables so that
                      other organism generate a benefit*/
                  
                  /* All the organism at a location are inputs into a function at that location, the location has a resource id that is used as a target for that location function. The location will modify the organism gentics to optimize the location function. */
                  
                  //Using :     d1 =   sqrt((x - xo)^2 + (y-yo)^2 + (z-zo)^2)
                  //
                  //

                  loc_cum_res = 0;
                  for(org_idx = 0; org_idx < m_space[vert_loc][horz_loc].num_org; org_idx++)
                  {
                      //Compute the drain of resources at a location (D)
               
                      org_pop_idx = pop_idx[org_idx];
                      population[org_pop_idx].num_org_coloc = m_space[vert_loc][horz_loc].num_org;
          
                      population[org_pop_idx].rsrc_lvl.loc_drain = fabs( (double)
                                                            (((float)population[org_pop_idx].chromo[RSC_DRN_LVL_1]
                                                             * (float)population[org_pop_idx].chromo[RSC_DRN_LVL_1])
                                                              - ((float)m_space[vert_loc][horz_loc].rsrc_map[RSC_DRN_LVL_1]
                                                               *(float)m_space[vert_loc][horz_loc].rsrc_map[RSC_DRN_LVL_1])) +
                                                             (((float)population[org_pop_idx].chromo[RSC_DRN_LVL_2]
                                                              * (float)population[org_pop_idx].chromo[RSC_DRN_LVL_2])
                                                               - ((float)m_space[vert_loc][horz_loc].rsrc_map[RSC_DRN_LVL_2]
                                                                *(float)m_space[vert_loc][horz_loc].rsrc_map[RSC_DRN_LVL_2])) +
                                                                    (((float)population[org_pop_idx].chromo[RSC_DRN_LVL_3]
                                                                      * (float)population[org_pop_idx].chromo[RSC_DRN_LVL_3])
                                                                     - ((float)m_space[vert_loc][horz_loc].rsrc_map[RSC_DRN_LVL_3]
                                                                        *(float)m_space[vert_loc][horz_loc].rsrc_map[RSC_DRN_LVL_3]))
                                                                    );
                                                             
                                                             
                      population[org_pop_idx].rsrc_lvl.loc_drain = population[org_pop_idx].rsrc_lvl.loc_drain;
                      
                      //compute the benefit of resources at a location (B)
                      
                      population[org_pop_idx].rsrc_lvl.loc_benefit = fabs( (double)
                                                            ((float)(population[org_pop_idx].chromo[RSC_ADD_LVL_1]
                                                              * (float)population[org_pop_idx].chromo[RSC_ADD_LVL_1])
                                                             - ((float)m_space[vert_loc][horz_loc].rsrc_map[RSC_ADD_LVL_1]
                                                                *(float)m_space[vert_loc][horz_loc].rsrc_map[RSC_ADD_LVL_1])) +
                                                            (((float)population[org_pop_idx].chromo[RSC_ADD_LVL_2]
                                                              * (float)population[org_pop_idx].chromo[RSC_ADD_LVL_2])
                                                             - ((float)m_space[vert_loc][horz_loc].rsrc_map[RSC_ADD_LVL_2]
                                                                *(float)m_space[vert_loc][horz_loc].rsrc_map[RSC_ADD_LVL_2])) +
                                                                      (((float)population[org_pop_idx].chromo[RSC_ADD_LVL_3]
                                                                        * (float)population[org_pop_idx].chromo[RSC_ADD_LVL_3])
                                                                       - ((float)m_space[vert_loc][horz_loc].rsrc_map[RSC_ADD_LVL_3]
                                                                          *(float)m_space[vert_loc][horz_loc].rsrc_map[RSC_ADD_LVL_3]))
                                                                      );
                      
                      population[org_pop_idx].rsrc_lvl.loc_benefit = population[org_pop_idx].rsrc_lvl.loc_benefit;
                      population[org_pop_idx].loc_net_rsc = population[org_pop_idx].rsrc_lvl.loc_benefit - population[org_pop_idx].rsrc_lvl.loc_drain;
#if 0
                      if(isnan(population[org_pop_idx].loc_net_rsc))
                      {
                      printf("RD- %u %u %u %u %u %u ", population[org_pop_idx].chromo[RSC_DRN_LVL_1], population[org_pop_idx].chromo[RSC_DRN_LVL_2],
                             population[org_pop_idx].chromo[RSC_DRN_LVL_3],
                             m_space[vert_loc][horz_loc].rsrc_map[RSC_DRN_LVL_1],
                             m_space[vert_loc][horz_loc].rsrc_map[RSC_DRN_LVL_2],
                             m_space[vert_loc][horz_loc].rsrc_map[RSC_DRN_LVL_3]);
                     
                      printf("RA- %u %u %u %u %u %u ", population[org_pop_idx].chromo[RSC_ADD_LVL_1], population[org_pop_idx].chromo[RSC_ADD_LVL_2],
                             population[org_pop_idx].chromo[RSC_ADD_LVL_3],
                             m_space[vert_loc][horz_loc].rsrc_map[RSC_ADD_LVL_1],
                             m_space[vert_loc][horz_loc].rsrc_map[RSC_ADD_LVL_2],
                             m_space[vert_loc][horz_loc].rsrc_map[RSC_ADD_LVL_3]);
                          
                          printf("Problem %d %d %d\r\n", population[org_pop_idx].rsrc_lvl.loc_benefit , population[org_pop_idx].rsrc_lvl.loc_drain, population[org_pop_idx].loc_net_rsc);
                      }
                          
#endif
                      

                      
                      
                          

                      
    //                 printf("Main THread B- %f D- %f N- %f %u %u\n", population[org_pop_idx].rsrc_lvl.loc_benefit, population[org_pop_idx].rsrc_lvl.loc_drain, population[org_pop_idx].loc_net_rsc,vert_loc ,horz_loc );
                      //compute the net value of the location for the organism
                      //if just one organism is at a location then the location net difference is T = (B - D)

                      loc_cum_res += population[org_pop_idx].loc_net_rsc;
                      //if two organisms are at a location the the total is T = ((B1 + B2) - (D1+ D2))
                      
                  }
                  
              }
        }
        
        //Next Compute the population wide operations
        
        set_value_out = 0;
        printf("Transversed the map %llu\n", current_sim_time);
        usleep(1000000);
        for(org_idx = 0; org_idx < POPULATION_SZ; org_idx++)
        {
            population[org_idx].pop_idx = org_idx;

            OrganismPrfile(&population[org_idx],NULL);
            
           if(population[org_idx].next_horz_loc != population[org_idx].horz_loc && population[org_idx].next_vert_loc != population[org_idx].vert_loc)
            {
                set_value_out = 1;
        //     printf("Moving %i to %u %u \r\n",org_idx, population[org_idx].next_horz_loc,population[org_idx].next_vert_loc );
                population[org_idx].horz_loc = population[org_idx].next_horz_loc;
                population[org_idx].vert_loc = population[org_idx].next_vert_loc;
            }
            
            if(population[org_idx].energy_lvl < 0.001)
            {
         //       printf("Dead at idx %i \r\n", org_idx);
                population[org_idx].vert_loc = 0;
                population[org_idx].horz_loc = 0;
            }
            
            
            
            msg_out[org_idx].horz_loc = population[org_idx].horz_loc;
            msg_out[org_idx].vert_loc = population[org_idx].vert_loc;
  //          printf("Energy at %i is %f \r\n", org_idx, population[org_idx].energy_lvl );
        }
        
        
        printf("Out Message %i %i %i\r\n",msg_out[0].horz_loc, msg_out[org_idx].vert_loc, set_value_out );
        
        current_sim_time++;
    }
    
   exit(0);
}


void * terminate_all(int  arg1, char * arg2 )
{
    
    printf("Caught a termination from line %i file %s \n", arg1  , arg2 );
    
 /*   for(i = 0; i< thread_cnt; i++)
    {
        printf("Killing thread %i \n", i);
        if(pthread_cancel(threads[i]))
        {
            printf("Thread failed to cancel \n");
        }
        else
        {
            printf("Canceled the %i thread \n", i);
        }
    }
  */
    
    printf("Ending the process \n");
    exit(-1);
    
}



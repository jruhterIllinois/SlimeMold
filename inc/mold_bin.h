/* 
 * File:   mold_bin.h
 * Author: jay
 *
 * Created on September 27, 2011, 8:40 PM
 */

#ifndef _MOLD_BIN_H
#define	_MOLD_BIN_H

#ifdef	__cplusplus
extern "C" {
#endif



    
    
void * terminate_all(int  arg1, char * arg2 );
float sigmoid_func(float * in, float beta, float offset);
    
void * ANNet(void * args);
void * OrganismPrfile(void * org, void * args2);
void * computeOrganismEnergy(void * arg1, void * args2);
    void find_new_org_loc(uint16_t  v, uint16_t  h, uint16_t * vs, uint16_t * hs);
    
void * ANNet_initialize(void * args);
float slime_fact(uint8_t  in);

//void Cluster_learn (void * arg);
//void * competitive_learn(void * arg);
//void * perceptron_learn (void * in, void * in2,  int train_seq);


#ifdef	__cplusplus
}
#endif

#endif	/* _MOLD_BIN_H */


#ifndef __PAT_PMT_H__
#define __PAT_PMT_H__

#include <stdint.h>

typedef struct {
	uint32_t program_clock_reference_base_32_1;
	uint16_t program_clock_reference_base_0 : 1;
	uint16_t reserved_5_0 : 6;
	uint16_t program_clock_reference_extension : 9;
}PCR_INFO;
typedef struct {
	uint8_t adaptation_field_length;
	uint8_t discontinuity_indicator : 1;
	uint8_t random_access_indicator : 1;
	uint8_t elementary_stream_priority_indicator : 1;
	uint8_t PCR_flag : 1;
	uint8_t OPCR_flag : 1;
	uint8_t splicing_point_flag : 1;
	uint8_t transport_private_data_flag : 1;
	uint8_t adaptaion_field_extension_flag : 1;
	PCR_INFO pcr_info;
}ts_adaption_list;

typedef struct 
{
uint16_t program_num ;
uint16_t pmt_pid;
}program_list ;


typedef struct 
{
	uint8_t  total;
	program_list program[20] ;  //the max stream in one ts
}pat_list ;


pat_list patparse(uint8_t *buf) ;

typedef struct {

	uint8_t  stream_type ;
	uint16_t es_pid ;

}es_list ;

typedef struct {
	uint16_t program_num ;
	uint16_t pcr_pid ;
	uint8_t  total_es ;
	es_list alles[5] ; // video audio teltext,subtitle
}pmt_list;

pmt_list pmtparse(uint8_t *buf,uint16_t pmt_pid) ;

#endif


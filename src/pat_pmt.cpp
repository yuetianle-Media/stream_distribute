#include "pat_pmt.h"
#include "demux.h"
#include <string.h>

pat_list patparse(uint8_t *buf)
{
	TSPacket_t data;
	pat_list programs ;
	uint16_t adaption = 0 ;
	uint16_t program_num = 0 ;
	uint16_t program_map_pid = 0 ;
	uint16_t section_length = 0 ;
	uint8_t payload_start = 0;
	uint8_t pointer = 0;
	uint8_t table_id = 0;
	uint8_t *p = 0;
        uint8_t *payload_data = NULL;

	memcpy((void*)&data,buf,sizeof(data));
	memset((void*)&programs,0,sizeof(programs));
	if(TSPACKET_ISPAYLOADUNITSTART(data))
	{
		adaption = TSPACKET_GETADAPTATION(data);
		if(adaption&0x01)
		{
			//printf("pid 0 has payload\n");
//                         if(adaption&0x02)  //has adaption filed
//                         {
//                                 payload_start = data.payload[0] + 1;
//                                 pointer = data.payload[payload_start];
//                                 payload_start += pointer ;
//                                 payload_start++;
//                                 //printf("has adaption payload start %d\n",payload_start);
//                         }
//                         else 
//                         {
//                                 pointer = data.payload[payload_start];
//                                 payload_start += pointer ;
//                                 payload_start++;
//                         }
                        /* Return if no payload in the TS packet */
                        if(!(buf[3] & 0x10))
                        {
                                return programs;
                        }

                        /* Skip the adaptation_field if present */
                        if(buf[3] & 0x20)
                                payload_data = buf + 5 + buf[4];
                        else
                                payload_data = buf + 4;

                        /* Unit start -> skip the pointer_field and a new section begins */
                        if(buf[1] & 0x40)
                        {
                                payload_data = payload_data + *payload_data + 1;
                        }
                        payload_start = payload_data - buf -4;
			//printf("no adaption payload start %d\n",payload_start);
			table_id = data.payload[payload_start];
			if(table_id)   // not pat table
				return programs;
			payload_start += 1;  //skip tableid
			p = &data.payload[payload_start];
			section_length = GET2BYTES(p);
			section_length = section_length & 0x03ff ; //10bit section length
			section_length -= 9 ;
			payload_start += 7 ;
				
			while(section_length)
			{
				p = &data.payload[payload_start];
				program_num = GET2BYTES(p);
				programs.program[programs.total].program_num= program_num ;
				//printf("progam number==== %x \n",program_num);
				payload_start += 2;
				p = &data.payload[payload_start];
				program_map_pid = GET2BYTES(p) & 0x1fff;
				programs.program[programs.total].pmt_pid = program_map_pid ;
				programs.total += 1 ;
				payload_start += 2;
				//printf("program map pid==== %x\n",program_map_pid);
				section_length-=4;
			}
		}
	}
	return programs;
}

pmt_list pmtparse(uint8_t *buf,uint16_t pmt_pid)
{
	TSPacket_t data;
	pmt_list pmt ;
	uint16_t adaption = 0 ;
	uint16_t section_length = 0;
	uint16_t program_info_length = 0 ;
	uint16_t es_info_length = 0 ;
	uint8_t payload_start = 0;
	uint8_t pointer = 0;
	uint8_t table_id = 0;
	uint8_t *p = 0;
        uint8_t *payload_data = NULL;

	memcpy((void*)&data,buf,sizeof(data));
	memset((void*)&pmt,0,sizeof(pmt));
	if(TSPACKET_ISPAYLOADUNITSTART(data))
	{
		adaption = TSPACKET_GETADAPTATION(data);
		if(adaption&0x01)
		{
			if(!(buf[3] & 0x10))
                        {
                                return pmt;
                        }

                        /* Skip the adaptation_field if present */
                        if(buf[3] & 0x20)
                                payload_data = buf + 5 + buf[4];
                        else
                                payload_data = buf + 4;

                        /* Unit start -> skip the pointer_field and a new section begins */
                        if(buf[1] & 0x40)
                        {
                                payload_data = payload_data + *payload_data + 1;
                        }
                        payload_start = payload_data - buf -4;
                        
			// printf("no adaption payload start %d\n",payload_start);
			table_id = data.payload[payload_start];
			if(table_id != 0x02)
				return pmt ;
			payload_start += 1;  //skip tableid
			p = &data.payload[payload_start];
			section_length = GET2BYTES(p);
			section_length = section_length & 0x03ff ; //10bit section length
			// printf("section length %x\n",section_length);
			payload_start += 2 ;
			p = &data.payload[payload_start];
			pmt.program_num = GET2BYTES(p);
			// printf("program num %x\n",pmt.program_num);
			payload_start += 5 ;
			p = &data.payload[payload_start];
			pmt.pcr_pid = GET2BYTES(p) &0x1fff;
			// printf("pcr pid %x\n",pmt.pcr_pid);
			payload_start += 2 ;
			p = &data.payload[payload_start];
			program_info_length = GET2BYTES(p) &0x0fff;
			// printf("program info length %x\n",program_info_length);
			payload_start += 2;
			payload_start += program_info_length ;
				
			section_length-=13;
			section_length-=program_info_length;
			// printf("now section length %x\n",section_length);
			while (section_length)
			{
				pmt.alles[pmt.total_es].stream_type = data.payload[payload_start];
				// printf("stream_type %x\n",pmt.alles[pmt.total_es].stream_type);
				payload_start += 1;
				p = &data.payload[payload_start];
				pmt.alles[pmt.total_es].es_pid = GET2BYTES(p) &0x1fff;
				// printf("es_pid %x\n",pmt.alles[pmt.total_es].es_pid);
				pmt.total_es++;
				payload_start += 2;
				p = &data.payload[payload_start];
				es_info_length = GET2BYTES(p) &0x0fff;
				// printf("es_info_length %x\n",es_info_length);
				payload_start += 2;
				payload_start += es_info_length; 
				section_length -= 5 ;
				section_length -= es_info_length; 
			}
		}
	} //end payload start
	return pmt;
}

ts_adaption_list ts_adpatation_parser(uint8_t *data)
{
	ts_adaption_list ts;
	return ts;
}


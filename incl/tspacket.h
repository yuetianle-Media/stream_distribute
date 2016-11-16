#pragma once
#include "dvb.h"
#include "ztypes.h"

#define MPEG_FRAME_TYPE_I	1
#define MPEG_FRAME_TYPE_P	2
#define MPEG_FRAME_TYPE_B	3

inline bool get_pcr(const unsigned char* p_ts_packet, PCR* p_pcr, int* p_pcr_pid)
{
	if ((p_ts_packet[0] == 0x47) &&
		(p_ts_packet[3] & 0x20) &&
		(p_ts_packet[5] & 0x10) &&
		(p_ts_packet[4] >= 7))
	{
		*p_pcr_pid = ((int)p_ts_packet[1] & 0x1F) << 8 | p_ts_packet[2];

		*p_pcr = ((int64_t)p_ts_packet[6] << 25) |
			((int64_t)p_ts_packet[7] << 17) |
			((int64_t)p_ts_packet[8] << 9) |
			((int64_t)p_ts_packet[9] << 1) |
			((int64_t)p_ts_packet[10] >> 7);
		return true;
	}
	return false;
}
class CTsPacket
{
public:
	CTsPacket(void);
	~CTsPacket(void);

	 bool SetPacket(BYTE *pPacket);

	 void Set_PID(PID pid);
	 bool Set_PCR(PCR pcr);
	 void Set_continuity_counter(BYTE Counter);
	 void RemoveAfField();
	 bool Get_transport_error_indicator();
	 bool Get_payload_unit_start_indicator();
	 bool Get_transport_priority();
	 WORD Get_PID();
	 BYTE Get_transport_scrambling_control();
	 BYTE Get_adaptation_field_control();
	 BYTE Get_continuity_counter();

	 BYTE * Get_adaptation_field(BYTE &Size);
	 BYTE Get_adaptation_field_length(BYTE *pAF_Data);
	 bool Get_discontinuity_indicator(BYTE *pAF_Data);
	 bool Get_random_access_indicator(BYTE *pAF_Data);
	 bool Get_elementary_stream_priority_indicator(BYTE *pAF_Data);
	 bool Get_PCR_flag(BYTE *pAF_Data);
	 bool Get_PCR_flag();
	 bool Get_OPCR_flag(BYTE *pAF_Data);
	 bool Get_splicing_point_flag(BYTE *pAF_Data);
	 bool Get_transport_private_data_flag(BYTE *pAF_Data);
	 bool Get_adaptation_field_extension_flag(BYTE *pAF_Data);
	 LONGLONG Get_program_clock_reference_base(BYTE *pAF_Data);
	 WORD Get_program_clock_reference_extension(BYTE *pAF_Data);
	 PCR  Get_PCR(BYTE *pAF_Data);
	 PCR  Get_PCR();

	 LONGLONG Get_original_program_clock_reference_base(BYTE *pAF_Data);
	 WORD Get_original_program_clock_reference_extension(BYTE *pAF_Data);
	 PCR  Get_OPCR(BYTE *pAF_Data);
	 BYTE Get_splice_countdown(BYTE *pAF_Data);
	 BYTE Get_transport_private_data_length(BYTE *pAF_Data);
	 BYTE *Get_private_data_byte(BYTE *pAF_Data,int &size);
	 BYTE Get_adaptation_field_extension_length(BYTE *pAF_Data);
	 bool Get_ltw_flag(BYTE *pAF_Data);
	 bool Get_piecewise_rate_flag(BYTE *pAF_Data);
	 bool Get_seamless_splice_flag(BYTE *pAF_Data);
	 bool Get_ltw_valid_flag(BYTE *pAF_Data);
	 WORD Get_ltw_offset(BYTE *pAF_Data);
	 DWORD Get_piecewise_rate(BYTE *pAF_Data);
	 LONGLONG Get_DTS_next_AU(BYTE *pAF_Data);
	 BYTE *Get_stuffing_byte(BYTE *pAF_Data,BYTE &Size);

	 bool Get_PES_stream_id(BYTE &stream_id);
	 bool	Get_PES_GOP_INFO(DWORD &TimeCode/*ʱ����Ϣ*/);
	 bool	Get_PES_PIC_INFO(BYTE &PictureType/*ʱ����Ϣ*/);

	 BYTE * Get_Data(BYTE &Size);
	
	BYTE *m_pPacket;
};


class CPrivate_Section
{
public:
	CPrivate_Section();
	~CPrivate_Section();
	
	bool SetPacket(CTsPacket &Packet);
	
	BYTE Get_table_id();
	bool Get_Section_sytax_indicator();
	WORD Get_Section_length();
	WORD Get_table_id_extension();
	BYTE Get_version_number();
	bool Get_current_next_indicator();
	BYTE Get_section_number();
	BYTE Get_last_section_number();

	BYTE * Get_private_data_byte(WORD &Size);
	
	DWORD Get_CRC_32();
	
private:
    BYTE *m_pSection;
};


class CDescriptor
{
public:
	CDescriptor(){};
	~CDescriptor(){};
	void SetDescriptor(BYTE *Descriptor);
	BYTE Get_descriptor_tag();
	BYTE Get_descriptor_length();
private:
	BYTE *m_pDescriptor;
};

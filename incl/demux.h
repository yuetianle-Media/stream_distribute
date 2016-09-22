#ifndef _DEMUX_H
#define _DEMUX_H



#define GET2BYTES(start) ((start)[0]<<8 | ((start)[1]))
#define GET3BYTES(start) (0<<24 |(start)[0]<<16 | (start)[1]<<8 | ((start)[2]))
#define GETWORD(start) ((start)[0]<<24 | (start)[1]<<16 | (start)[2]<<8 | ((start)[3]))

#define Nonexistent  0
#define CurrentPacket 1
#define LastPacket 2
#define FindPart 3

#define SEARCH_I_START 0x01
#define SEARCH_P_START 0x02
#define SEARCH_B_START 0x03
#define SEARCH_FRAME_START 0xFF
#define SEARCH_SEQUENCE_START 0xFE


typedef struct TSPacket_t
{
    uint8_t header[4];                  /**< Packet Header fields */
    uint8_t payload[184]; /**< Data contained in the packet */
}
TSPacket_t;

typedef struct startcode_info
{
    uint8_t start_exist_info; /*0:no find;  1:find;  2:startcode separate into 2 part 3:only find a part now*/
    uint8_t startcode_pos;    /*when start_exist_info=1,return offset in payload;when start_exist_info=12,return 1th part length*/
    uint8_t find_part_len;    /*when find part of start,record part len*/
    uint8_t payload_pos;      /*payload position in ts packet */
    uint8_t start_type;		/*0:none 1:I 2:P 3:B 0xB3:sequence */
}
startcode_info;

startcode_info searchstartcode(TSPacket_t *data,uint16_t videopid,uint8_t search_type,uint8_t find_part_len);  //only check the video pes
/**
 * Retrieves the PID of packet from the packet header
 * @param packet The packet to extract the PID from.
 * @return The PID of the packet as a 16bit integer.
 */
#define TSPACKET_GETPID(packet) \
    ((((packet).header[1] & 0x1f) << 8) | (packet).header[2])

/**
 * Sets the PID of the packet in the packet header.
 * @param packet The packet to update.
 * @param pid    The new PID to set.
 */
#define TSPACKET_SETPID(packet, pid) \
    do{ \
        (packet).header[1] = ((packet).header[1] & 0xe0) | ((pid >> 8) & 0x1f); \
        (packet).header[2] = pid & 0xff; \
    }while(0)
/**
 * Retrieves the packet sequence count.
 * @param packet The packet to extract the count from.
 * @return The packet sequence count as a 4 bit integer.
 */
#define TSPACKET_GETCOUNT(packet) \
    ((packet).header[3] & 0x0f)

/**
 * Sets the packet sequence count.
 * @param packet The packet to update.
 * @param count  The new sequence count to set.
 */
#define TSPACKET_SETCOUNT(packet, count) \
    ((packet).header[3] = ((packet).header[3] & 0xf0) | ((count) & 0x0f))

/**
 * Boolean test to determine whether this packet is the start of a payload.
 * @param packet The packet to check.
 */
#define TSPACKET_ISPAYLOADUNITSTART(packet) \
    (((packet).header[1] & 0x40) == 0x40)
/**
 * Boolean test to determine whether this packet is valid, transport_error_indicator 
 * is not set.
 * @param packet The packet to check.
 * @return True if the packet is valid, false otherwise.
 */
#define TSPACKET_ISVALID(packet) \
    (((packet).header[1] & 0x80) == 0x00)

/**
 * Retrieves the priority field of the packet.
 * @param packet The packet to check.
 * @return The packet priority.
 */
#define TSPACKET_GETPRIORITY(packet) \
    (((packet).header[1] & 0x20) >> 4)

/**
 * Set the priority field of the packet.
 * @param packet The packet to update.
 * @param priority Either 1 or 0 to indicate that this is a priority packet.
 */
#define TSPACKET_SETPRIORITY(packet, priority) \
    ((packet).header[1] = ((packet).header[1] & 0xdf) | (((priority) << 4) & 0x20))

/**
 * Retrieve whether the packet has an adaptation field.
 * @param packet The packet to check.
 * @return The adapatation field control flags.
 */
#define TSPACKET_GETADAPTATION(packet) \
    (((packet).header[3] & 0x30) >> 4)

/**
 * whether the packet has an pcr .
 * @param packet The packet to check.
 * @return True if pcr flags is set,false otherwise.
 */
#define TSPACKET_ISPCRFLAGSET(packet) \
    (((packet).payload[1] & 0x10) == 0x10)
/**
 * Retrieve an pcr base field.
 * @param packet The packet to check.
 * @return The pcr field base.
 */
#define TSPACKET_GETPCRBASE_32(packet) \
    (((packet).payload[2]) << 24|((packet).payload[3]) <<16 |((packet).payload[4]) <<8|(packet).payload[5])
/**
 * Retrieve an pcr extension field.
 * @param packet The packet to check.
 * @return The pcr field base.
 */
#define TSPACKET_GETPCREXT(packet) \
    ((packet).payload[7]) | (((packet).payload[6] & 0x01)<<8)
/**
 * Set whether the packet has an adaptation field.
 * @param packet The packet to update.
 * @param adaptation The new adaptation field flags.
 */
#define TSPACKET_SETADAPTATION(packet, adaptation) \
    ((packet).header[3] = ((packet).header[3] & 0xcf) | (((adaptation) << 4) & 0x30))

/**
 * Retrieves the adaptation field length.
 * @param packet The packet to check.
 * @return The length of the adaptation field.
 */
#define TSPACKET_GETADAPTATION_LEN(packet) \
    ((packet).payload[0])

#define PESPACKET_GETPTS_32BIT(pdata) \
	((*(pdata)&0x0E)<<28|*(pdata+1)<<21|(*(pdata+2)&0xFE)<<13|*(pdata+3)<<6|(*(pdata+4)&0xFC)>>2)


#endif


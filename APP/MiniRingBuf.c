/*
	File:    MiniRingBuf.c
	Author:  Light&Electricity
	Date:    2025.6.27
	Version: 0.5
*/
#include "MiniRingBuf.h"

/* init mrb struct */
void mrb_init(MiniRingBuf *mrb, MRB_TYPE_BUF *buf, MRB_TYPE_SIZE size, MRB_TYPE_BYTE set)
{
	mrb->start = mrb->end = 0;
	mrb->buf = buf;
	mrb->size = size;
	mrb->set = set;
#if (MRB_MUTEX_EN)
	mrb->mutex = (void*)0; // NULL
#endif
#if (MRB_CALLBACK_ANY)
	mrb->callback = (void*)0; // NULL
#endif
}

/* clear entire buf */
void mrb_clear(MiniRingBuf *mrb)
{
	MRB_clear(mrb);
}

/* check if it's empty */
MRB_TYPE_BOOL mrb_empty(MiniRingBuf *mrb)
{
	return MRB_empty(mrb);
}

/* check if it's full */
MRB_TYPE_BOOL mrb_full(MiniRingBuf *mrb)
{
	MRB_TYPE_BOOL ret;
#if (MRB_MUTEX_EN)
	MRB_MUTEX_LOCK(mrb);
#endif
#if (MRB_CRITICAL_EN)
	MRB_CRITICAL_START(mrb);
#endif

	ret = MRB_full(mrb);

#if (MRB_CRITICAL_EN)
	MRB_CRITICAL_END(mrb);
#endif
#if (MRB_MUTEX_EN)
	MRB_MUTEX_UNLOCK(mrb);
#endif

	return ret;
}

/* get used length */
MRB_TYPE_SIZE mrb_len(MiniRingBuf *mrb)
{
	MRB_TYPE_SIZE tmpLen;
#if (MRB_SISO_SAFE)
	MRB_TYPE_SIZE tmpEnd;
#endif

#if (MRB_MUTEX_EN)
	MRB_MUTEX_LOCK(mrb);
#endif
#if (MRB_CRITICAL_EN)
	MRB_CRITICAL_START(mrb);
#endif

#if (MRB_SISO_SAFE) // get current length safely
	tmpLen = mrb->start;
	tmpEnd = mrb->end;
	tmpLen = MRB_len_calculate(mrb->size, tmpLen, tmpEnd);
#else
	tmpLen = MRB_len(mrb); // get current length quickly
#endif

#if (MRB_CRITICAL_EN)
	MRB_CRITICAL_END(mrb);
#endif
#if (MRB_MUTEX_EN)
	MRB_MUTEX_UNLOCK(mrb);
#endif

	return tmpLen;
}

/* delete items */
MRB_TYPE_USE mrb_del(MiniRingBuf *mrb, MRB_TYPE_USE len)
{
	MRB_TYPE_SIZE tmpLen;
#if (MRB_SISO_SAFE)
	MRB_TYPE_SIZE tmpEnd;
#endif

#if (MRB_MUTEX_EN)
	MRB_MUTEX_LOCK(mrb);
#endif
#if (MRB_CRITICAL_EN)
	MRB_CRITICAL_START(mrb);
#endif

#if (MRB_SISO_SAFE) // get current length safely
	tmpEnd = mrb->end; // get END first to make a conservative estimate of length
	tmpLen = mrb->start;
	tmpLen = MRB_len_calculate(mrb->size, tmpLen, tmpEnd);
#else
	tmpLen = MRB_len(mrb); // get current length quickly
#endif

	if(tmpLen < len) len = tmpLen; // no enough data, only delete the existing data
	mrb->start += len; // move START to delete
	if(mrb->start >= mrb->size) mrb->start -= mrb->size;

#if (MRB_CALLBACK_EMPTY)
	if(mrb->callback && MRB_empty(mrb)) mrb->callback(mrb, MRB_CALLBACK_EMPTY);
#endif

#if (MRB_CRITICAL_EN)
	MRB_CRITICAL_END(mrb);
#endif
#if (MRB_MUTEX_EN)
	MRB_MUTEX_UNLOCK(mrb);
#endif

	return len; // return the length of successfully delete data
}

/* copy from mrb without deleting */
MRB_TYPE_USE mrb_copy(MiniRingBuf *mrb, MRB_TYPE_BUF *buf, MRB_TYPE_USE len)
{
	MRB_TYPE_SIZE tmpLen;
#if (MRB_SISO_SAFE)
	MRB_TYPE_SIZE tmpEnd;
#endif

#if (MRB_MUTEX_EN)
	MRB_MUTEX_LOCK(mrb);
#endif
#if (MRB_CRITICAL_EN)
	MRB_CRITICAL_START(mrb);
#endif

#if (MRB_SISO_SAFE) // get current length safely
	tmpEnd = mrb->end; // get END first to make a conservative estimate of length
	tmpLen = mrb->start;
	tmpLen = MRB_len_calculate(mrb->size, tmpLen, tmpEnd);
#else
	tmpLen = MRB_len(mrb); // get current length quickly
#endif

	if(tmpLen < len){
#if (MRB_CALLBACK_NODATA)
		if(mrb->callback) mrb->callback(mrb, MRB_CALLBACK_NODATA);
#endif
		len = tmpLen; // no enough data, only read the existing data
	}
	else tmpLen = len; // save len as return value

#if MRB_COPY_METHOD == MRB_COPY_METHOD_LOOP
	for(len = 0; len < tmpLen && mrb->start + len < mrb->size;){
		*buf++ = mrb->buf[mrb->start + len++];
	}
	while(len < tmpLen){
		*buf++ = mrb->buf[mrb->start + len++ - mrb->size];
	}
#elif MRB_COPY_METHOD == MRB_COPY_METHOD_MEMCPY || MRB_COPY_METHOD == MRB_COPY_METHOD_MRBCPY
	tmpLen = mrb->size - mrb->start;
	if(len < tmpLen){
		MRB_COPY_FUNC(buf, &mrb->buf[mrb->start], sizeof(MRB_TYPE_BUF) * len);
	}
	else{
		MRB_COPY_FUNC(buf, &mrb->buf[mrb->start], sizeof(MRB_TYPE_BUF) * tmpLen);
		MRB_COPY_FUNC(&buf[tmpLen], &mrb->buf[0], sizeof(MRB_TYPE_BUF) * (len - tmpLen));
	}
	tmpLen = len; // save len as return value
#else
	#error "MRB_COPY_METHOD ERROR!"
#endif

#if (MRB_CRITICAL_EN)
	MRB_CRITICAL_END(mrb);
#endif
#if (MRB_MUTEX_EN)
	MRB_MUTEX_UNLOCK(mrb);
#endif

	return tmpLen; // return the length of successfully read data
}

/* read from mrb */
MRB_TYPE_USE mrb_read(MiniRingBuf *mrb, MRB_TYPE_BUF *buf, MRB_TYPE_USE len)
{
	MRB_TYPE_SIZE tmpLen;
#if (MRB_SISO_SAFE)
	MRB_TYPE_SIZE tmpEnd;
#endif

#if (MRB_MUTEX_EN)
	MRB_MUTEX_LOCK(mrb);
#endif
#if (MRB_CRITICAL_EN)
	MRB_CRITICAL_START(mrb);
#endif

#if (MRB_SISO_SAFE) // get current length safely
	tmpEnd = mrb->end; // get END first to make a conservative estimate of length
	tmpLen = mrb->start;
	tmpLen = MRB_len_calculate(mrb->size, tmpLen, tmpEnd);
#else
	tmpLen = MRB_len(mrb); // get current length quickly
#endif

	if(tmpLen < len){
#if (MRB_CALLBACK_NODATA)
		if(mrb->callback) mrb->callback(mrb, MRB_CALLBACK_NODATA);
#endif
		len = tmpLen; // no enough data, only read the existing data
	}
	else tmpLen = len; // save len as return value

#if MRB_COPY_METHOD == MRB_COPY_METHOD_LOOP
	while(len--){ MRB_read_one(mrb, *buf++); }
#elif MRB_COPY_METHOD == MRB_COPY_METHOD_MEMCPY || MRB_COPY_METHOD == MRB_COPY_METHOD_MRBCPY
	tmpLen = mrb->size - mrb->start;
	if(len < tmpLen){
		MRB_COPY_FUNC(buf, &mrb->buf[mrb->start], sizeof(MRB_TYPE_BUF) * len);
		mrb->start += len;
	}
	else{
		MRB_COPY_FUNC(buf, &mrb->buf[mrb->start], sizeof(MRB_TYPE_BUF) * tmpLen);
		MRB_COPY_FUNC(&buf[tmpLen], &mrb->buf[0], sizeof(MRB_TYPE_BUF) * (len - tmpLen));
		mrb->start = len - tmpLen;
	}
	tmpLen = len; // save len as return value
#else
	#error "MRB_COPY_METHOD ERROR!"
#endif

#if (MRB_CALLBACK_EMPTY)
	if(mrb->callback && MRB_empty(mrb)) mrb->callback(mrb, MRB_CALLBACK_EMPTY);
#endif

#if (MRB_CRITICAL_EN)
	MRB_CRITICAL_END(mrb);
#endif
#if (MRB_MUTEX_EN)
	MRB_MUTEX_UNLOCK(mrb);
#endif

	return tmpLen; // return the length of successfully read data
}

/* write to mrb */
MRB_TYPE_USE mrb_write(MiniRingBuf *mrb, const MRB_TYPE_BUF *buf, MRB_TYPE_USE len)
{
	MRB_TYPE_SIZE tmpLen;
#if (MRB_SISO_SAFE)
	MRB_TYPE_SIZE tmpEnd;
#endif

#if (MRB_MUTEX_EN)
	MRB_MUTEX_LOCK(mrb);
#endif
#if (MRB_CRITICAL_EN)
	MRB_CRITICAL_START(mrb);
#endif

#if (MRB_SISO_SAFE) // get current length safely
	tmpLen = mrb->start;
	tmpEnd = mrb->end; // get END late to make a conservative estimate of space
	tmpLen = MRB_len_calculate(mrb->size, tmpLen, tmpEnd);
#else
	tmpLen = MRB_len(mrb); // get current length quickly
#endif

	if(tmpLen + len >= mrb->size){ // no enough space
#if MRB_CALLBACK_NOSPACE
		if(mrb->callback) mrb->callback(mrb, MRB_CALLBACK_NOSPACE);
#endif
		if(mrb->set == MRB_SET_SKIPWRITE) len = 0;
		else if(mrb->set == MRB_SET_PARTWRITE) len = mrb->size - tmpLen - 1;
		else if(mrb->set == MRB_SET_OVERWRITE){
			len = 0;
		}
	}
	tmpLen = len; // save len as return value

#if MRB_COPY_METHOD == MRB_COPY_METHOD_LOOP
	while(len--){ MRB_write_one(mrb, *buf++); }
#elif MRB_COPY_METHOD == MRB_COPY_METHOD_MEMCPY || MRB_COPY_METHOD == MRB_COPY_METHOD_MRBCPY
	tmpLen = mrb->size - mrb->end;
	if(len < tmpLen){
		MRB_COPY_FUNC(&mrb->buf[mrb->end], buf, sizeof(MRB_TYPE_BUF) * len);
		mrb->end += len;
	}
	else{
		MRB_COPY_FUNC(&mrb->buf[mrb->end], buf, sizeof(MRB_TYPE_BUF) * tmpLen);
		MRB_COPY_FUNC(&mrb->buf[0], &buf[tmpLen], sizeof(MRB_TYPE_BUF) * (len - tmpLen));
		mrb->end = len - tmpLen;
	}
	tmpLen = len; // save len as return value
#else
	#error "MRB_COPY_METHOD ERROR!"
#endif

#if (MRB_CALLBACK_FULL)
	if(mrb->callback && MRB_full(mrb)) mrb->callback(mrb, MRB_CALLBACK_FULL);
#endif

#if (MRB_CRITICAL_EN)
	MRB_CRITICAL_END(mrb);
#endif
#if (MRB_MUTEX_EN)
	MRB_MUTEX_UNLOCK(mrb);
#endif

	return tmpLen;
}

#if MRB_COPY_METHOD == MRB_COPY_METHOD_MRBCPY
/* a simple memcpy function for 32bit core */
void* mrb_memcpy(void *dest, const void *src, uint32_t n)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    if(((uintptr_t)d & 0x03) == ((uintptr_t)s & 0x03)){ // 4-byte aligned
        while(((uintptr_t)d & 0x03) && n > 0){ // copy unaligned bytes
			n--;
            *d++ = *s++;
        }

        // use uint32_t to copy
        uint32_t *dw = (uint32_t *)d;
        const uint32_t *sw = (const uint32_t *)s;
        uint32_t n_words = n >> 2; // n / 4
        for(uint32_t i = 0; i < n_words; i++) dw[i] = sw[i]; // copy words

        // update the pointer for remaining data
        d = (uint8_t *)(dw + n_words);
        s = (const uint8_t *)(sw + n_words);
        n &= 0x03; // n % 4
    }
    else if (((uintptr_t)d & 0x01) == ((uintptr_t)s & 0x01)) { // 2-byte aligned
        if(((uintptr_t)d & 0x01) && n > 0){ // copy unaligned byte
			n--;
            *d++ = *s++;
        }

        // use uint16_t to copy
        uint16_t *dw = (uint16_t *)d;
        const uint16_t *sw = (const uint16_t *)s;
        uint32_t n_words = n >> 1; // n / 2
        for (uint32_t i = 0; i < n_words; i++) dw[i] = sw[i]; // copy halfwords

        // update the pointer for remaining data
        d = (uint8_t *)(dw + n_words);
        s = (const uint8_t *)(sw + n_words);
        n &= 0x01; // n % 2
    }
	// else unaligned

    for(uint32_t i = 0; i < n; i++) d[i] = s[i]; // copy all remaining bytes

    return dest;
}
#endif






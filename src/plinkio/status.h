/**
 * Copyright (c) 2012-2013, Mattias Frånberg
 * All rights reserved.
 *
 * This file is distributed under the Modified BSD License. See the COPYING file
 * for details.
 */

#ifndef __STATUS_H__
#define __STATUS_H__

#ifdef __cplusplus
extern "C" {
#endif

enum pio_status_e
{
    /**
     * Function successful.
     */
    PIO_OK,

    /**
     * File reached EOF.
     */
    PIO_END,

    /**
     * Generic error.
     */
    PIO_ERROR,

    /**
     * FAM IO error.
     */
    P_FAM_IO_ERROR,

    /**
     * BIM IO error.
     */
    P_BIM_IO_ERROR,
    
    /**
     * Bed IO error.
     */
    P_BED_IO_ERROR
};

typedef enum pio_status_e pio_status_t;

#ifdef __cplusplus
}
#endif

#endif /* End of __STATUS_H__ */

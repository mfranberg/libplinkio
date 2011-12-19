#ifndef __STATUS_H__
#define __STATUS_H__

enum PioStatus
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
    PIO_ERROR
};

#endif /* End of __STATUS_H__ */

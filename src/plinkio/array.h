#define element_t int

/**
 * Simple dynamic array implementation.
 */
struct array_t
{
    /**
     * Current size of the array.
     */
    unsigned int size;

    /**
     * Maximum size of the array.
     */
    unsigned int max_size;

    /**
     * Elements in the array.
     */
    element_t *elements;
};

/**
 * Initialize the array, must be called before calls to
 * other array_* functions.
 *
 * @param array The array.
 * @param initialSize Number of elements in the inital array.
 */
void array_init(struct array_t *array, unsigned int initialSize)
{
    bzero( array, sizeof( struct array_t ) );
    
    array->size = initialSize;
    array->max_size = initialSize;
    array->elements = (element_t *) malloc( sizeof( element_t ) * initialSize );

    return;
}

/**
 * Adds an element to the array, grows it if necessary. Exits if array
 * cannot be realloced.
 *
 * @param The array.
 * @param Element to add.
 */
void array_add(struct array_t *array, element_t element)
{
    if( array->size == array->max_size )
    {
        element_t *new_elements = (element_t *) realloc( sizeof( element_t ) * array->max_size * 2 );
        if( new_elements != NULL )
        {
            array->elements = new_elements;
            array->max_size = array->max_size * 2;
        }
        else
        {
            fprintf( stderr, "array_add: error: Cannot realloc." );
            exit( 0 );
        }
    }

    array->elements[ size ] = element;
    size++;
}

/**
 * Returns an element at the given index.
 *
 * @param array The array.
 * @param index The index to get an element from.
 *
 * @return the element at the given index, null if no such element.
 */
element_t array_get(struct array_t *array, unsigned int index)
{
    return array->elements[ index ];
}

/**
 * Releases the memory allocated by the array an all its elements.
 *
 * @param array The array.
 */
void array_free(struct array_t *array)
{
    free( array->elements);
    bzero( array, sizeof( struct array_t ) );
}

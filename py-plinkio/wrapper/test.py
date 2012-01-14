import plinkio
pf = plinkio.open( "/mnt/peter/trans" )

for id, row in enumerate( pf ):
    print( id )



import megastructure as mega

# m = megastructure.Connection( "localhost", 1234 )

con = mega.Connection()

tf = con.TestFactory( con.mp() )
t = tf.create_test()
t.test1()

print( con  )

c = con.Connectivity( con.daemon() )
c.shutdown()


var nativeimage = require('./build/default/nativeimage')

var ni = new nativeimage.NativeImage();

//ni.combine4( 'Aldo', function(data){console.log(data)} )

var path = "/usr/src/fast-pyramid-generator/test_images/checkers/"
ni.combine4(
  path + "0_0.png", // top-left
  path + "1_0.png", // top-right
  path + "0_1.png", // bottom-left
  path + "1_1.png", // bottom-right
  "/tmp/test.jpg", // output image path
  function(data){ console.log(data) }
)

function c( destpath, cb ){
  ni.combine4(
    path + "0_0.png", // top-left
    path + "1_0.png", // top-right
    path + "0_1.png", // bottom-left
    path + "1_1.png", // bottom-right
    destpath, // output image path
    cb
  )
}

var total = 5 * 1000;
var i = 0;
var d1 = null;

var next = function(){
  c( '/tmp/stress/test' + i + '.jpg', function(){
    if( i%100 == 0 )
      console.log(i)
    if ( ++i < total ){
      next()
    } else {
      var d2 = new Date()
      console.log('done')
      console.log( d2.getTime() - d1.getTime() )
    }
  })
}

d1 = new Date()
next()

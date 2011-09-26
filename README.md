# Node NativeImage module: Minimal and Partial OpenCV Bindings for Node.js

These bindings only surface a tiny fraction of OpenCV functionality.
They were built to support a very specific project, and are thus not useful outside of its context.

Sorry ;)

# notes on building a node extension

https://www.cloudkick.com/blog/2010/aug/23/writing-nodejs-native-extensions/


converting arguments


http://stackoverflow.com/questions/7476145/converting-from-v8arguments-to-c-types


# Building and Dependencies

OpenCV

On a Mac

    sudo port install opencv

Put this lib in your includes dir

http://v8-juice.googlecode.com/files/libv8-convert-20110729.tar.gz


http://bravenewmethod.wordpress.com/2011/03/30/callbacks-from-threaded-node-js-c-extension/
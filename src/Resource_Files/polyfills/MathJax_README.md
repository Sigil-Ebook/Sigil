# mathjax version 3.22 #
https://github.com/mathjax/MathJax
MathJax Version 3.22 is the required minimum.

For embedded use in Sigil the files in this folder were 
a custom build from the official MathJax-3.22 release to 
create the custom-mathjax.min.js folder which is installed into Sigil.app
into a polyfills folder during the build.

The custom build includes the very minimum to fully support
mathml to svg conversion (including the mml3 extension).


The MathJax custom build was made following the official MathJax3 build from source instructions

    See "Making a Custom Build of MathJax"
    https://docs.mathjax.org/en/latest/web/webpack.html

with the following files:


custom-mathjax.js
-----------------
require('../components/src/startup/lib/startup.js');

const {Loader} = require('../js/components/loader.js');
Loader.preLoad(
    'loader',
    'startup',
    'core',
    '[mml]/mml3',
    'input/mml',
    'output/svg',
    'output/svg/fonts/tex.js'
);
    
require('../components/src/input/mml/extensions/mml3/mml3.js');
require('../components/src/core/core.js');
require('../components/src/input/mml/mml.js');
require('../components/src/output/svg/svg.js');
require('../components/src/output/svg/fonts/tex/tex.js');

const {insert} = require('../js/util/Options.js');
insert(MathJax.config, {
  mml: {
    packages: {'[+]': ['mml3']}
  }
});

require('../components/src/startup/startup.js');




webpack_config.js
-----------------

const PACKAGE = require('../components/webpack.common.js');

module.exports = PACKAGE(
  'custom-mathjax',                   // the package to build
  '../js',                            // location of the MathJax js library
  [],                                 // packages to link to
    __dirname,                        // our directory
  '.'                                 // where to put the packaged component
);

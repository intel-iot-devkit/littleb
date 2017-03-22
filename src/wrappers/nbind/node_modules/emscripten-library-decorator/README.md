emscripten-library-decorator
============================

This package provides decorators for writing Emscripten libraries.

```typescript
function _print(message: string) {
	console.log(message);
}

@exportLibrary
class test {
	@dep(_print)
	static hello() {
		_print('Hello, World!!');
	}

	static foobar = 42;
};
```

The class decorator `@exportLibrary` exports the static members of the class as an Emscripten library. Place it with no arguments just before the class.

The property decorator `@dep` is for listing dependencies. It ensures that when an exported function is used in the C or C++ code, other required functions are also included in the compiled Emscripten output after dead code elimination. Place it just before a function with any number of parameters listing the other required functions.

The dependencies should be global functions and their name should begin with an underscore. Otherwise Emscripten's name mangling will change their name in the output making any calls to them fail.

There is a [longer article](http://blog.charto.net/asm-js/Writing-Emscripten-libraries-in-TypeScript/) with more information.

API
===
Docs generated using [`docts`](https://github.com/charto/docts)
>
> <a name="api-defineHidden"></a>
> ### Function [`defineHidden`](#api-defineHidden)
> <em>@_defineHidden decorator.</em>  
> <em>Assign to a local variable called _defineHidden before using.</em>  
> <em>Apply to a property to protect it from modifications and hide it.</em>  
> Source code: [`<>`](http://github.com/charto/emscripten-library-decorator/blob/74128b8/index.ts#L107-L116)  
> > **defineHidden( )** <sup>&rArr; <code>(target: Object, key: string) =&gt; void</code></sup> [`<>`](http://github.com/charto/emscripten-library-decorator/blob/74128b8/index.ts#L107-L116)  
> > &emsp;&#x25ab; value<sub>?</sub> <sup><code>any</code></sup>  
>
> <a name="api-dep"></a>
> ### Function [`dep`](#api-dep)
> <em>@dep decorator.</em>  
> <em>Apply to a function, to list other required variables needing protection</em>  
> <em>from dead code removal.</em>  
> <em>Arguments can be functions or names of global variables.</em>  
> Source code: [`<>`](http://github.com/charto/emscripten-library-decorator/blob/74128b8/index.ts#L20-L48)  
> > **dep( )** <sup>&rArr; <code>(target: Object, functionName: string) =&gt; void</code></sup> [`<>`](http://github.com/charto/emscripten-library-decorator/blob/74128b8/index.ts#L20-L48)  
> > &emsp;&#x25aa; depList <sup><code>((...args: any[]) =&gt; any | string)[]</code></sup>  
>
> <a name="api-exportLibrary"></a>
> ### Function [`exportLibrary`](#api-exportLibrary)
> <em>@exportLibrary decorator.</em>  
> <em>Apply to a class with static methods, to export them as functions.</em>  
> Source code: [`<>`](http://github.com/charto/emscripten-library-decorator/blob/74128b8/index.ts#L53-L55)  
> > **exportLibrary( )** <sup>&rArr; <code>void</code></sup> [`<>`](http://github.com/charto/emscripten-library-decorator/blob/74128b8/index.ts#L53-L55)  
> > &emsp;&#x25aa; target <sup><code>any</code></sup>  
>
> <a name="api-prepareNamespace"></a>
> ### Function [`prepareNamespace`](#api-prepareNamespace)
> <em>@prepareNamespace decorator.</em>  
> <em>Apply to an empty, named dummy class defined at the end of the namespace</em>  
> <em>block, to prepare its contents for export in an Emscripten library.</em>  
> <em>Namespaces with matching names in different files are merged together.</em>  
> <em>All code in the block is separated because Emscripten only outputs global</em>  
> <em>functions, not methods.</em>  
> Source code: [`<>`](http://github.com/charto/emscripten-library-decorator/blob/74128b8/index.ts#L66-L77)  
> > **prepareNamespace( )** <sup>&rArr; <code>(target: any) =&gt; void</code></sup> [`<>`](http://github.com/charto/emscripten-library-decorator/blob/74128b8/index.ts#L66-L77)  
> > &emsp;&#x25aa; name <sup><code>string</code></sup>  
>
> <a name="api-publishNamespace"></a>
> ### Function [`publishNamespace`](#api-publishNamespace)
> <em>Call once per namespace at the global level, after all files with contents</em>  
> <em>in that namespace have been imported. Clears the namespace and exports a</em>  
> <em>"postset" function to populate it using its original code.</em>  
> Source code: [`<>`](http://github.com/charto/emscripten-library-decorator/blob/74128b8/index.ts#L83-L101)  
> > **publishNamespace( )** <sup>&rArr; <code>void</code></sup> [`<>`](http://github.com/charto/emscripten-library-decorator/blob/74128b8/index.ts#L83-L101)  
> > &emsp;&#x25aa; name <sup><code>string</code></sup>  
>
> <a name="api-setEvil"></a>
> ### Function [`setEvil`](#api-setEvil)
> <em>Allow decorators to call eval() in the context that called them.</em>  
> <em>This is needed for various transformations.</em>  
> Source code: [`<>`](http://github.com/charto/emscripten-library-decorator/blob/74128b8/index.ts#L11-L13)  
> > **setEvil( )** <sup>&rArr; <code>void</code></sup> [`<>`](http://github.com/charto/emscripten-library-decorator/blob/74128b8/index.ts#L11-L13)  
> > &emsp;&#x25aa; otherEval <sup><code>(code: string) =&gt; any</code></sup> <em>must be this function: (code: string) => eval(code)</em>  

License
-------
[The MIT License](https://raw.githubusercontent.com/charto/emscripten-library-decorator/master/LICENSE)

Copyright (c) 2015-2016 BusFaster Ltd

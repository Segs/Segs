This document is more of a statement of intent, then statement of fact :)
***
1. Use spaces instead of tabs. ( Most editors allow one to set this up so 'tab' = 4 spaces )
2. The target brace style is: 
<pre lang="cpp"><code>
    namespace L
    {

    void exampleFunction(int params)
    {
        if (pred)
        {
            statement;
            statement;
        }
        else
        {
            while (pred) 
            {
                statement;
                statement;
            }
        }
    }

    }</code></pre>

3. Lines should be limited to 120 characters; break longer lines.
If an if statement has to be broken, use an extra tab on the continuation line e.g.
<pre lang="cpp"><code>
    if (very very very very very very very very very very very very
            very very long predicate)
    {
        statement; ...
</code></pre>
4. In header files, reserve 2 tabs for the optional keywords virtual or static,
and reserve enough tabs for the longest return type. i.e:
<pre lang="cpp"><code>
    class X
    {
    public:
                        X();
                        ~X();
    virtual SomeType *  function();
    static  void        otherfunction();
    };
</code></pre>

5. Use descriptive function/member names ( autocompletion is your friend )

6. Doxygen Comments - IMHO The place for doxygen comments is in the source file.
Headers should be clean of comments, as to allow for quick class interface scanning


This document is more of a statement of intent, then statement of fact :)
***
1. Use spaces instead of tabs. ( Most editors allow one to set this up so 'tab' = 4 spaces )
2. The target brace style is: 
```cpp
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

    }
```

3. Lines should be limited to 120 characters; break longer lines.
If an if statement has to be broken, use an extra tab on the continuation line e.g.
```cpp
    if (very very very very very very very very very very very very
            very very long predicate)
    {
        statement; ...
```
4. In header files, reserve 2 tabs for the optional keywords virtual or static,
and reserve enough tabs for the longest return type. i.e:
```cpp
    class X
    {
    public:
                        X();
                        ~X();
    virtual SomeType *  function();
    static  void        otherfunction();
    };
```

5. Use descriptive function/member names (autocompletion is your friend). Also, consider naming your variable with the following formats for consistency and readability:
```cpp
    #DEFINE     DEFINE_VAR                      // for DEFINE variables
    auto        m_member_var;                   // for member variables
    auto        s_file_scope_var;               // for file scope variables
    auto        g_global_var;                   // for global variables.
    auto        throw_away;                     // for single-use limited scope, or throwaway varables.
    QString     m_underscore_separated_words;   // underscore separate words
    int         s_dont_identify_type_in_name;   // don't identify the variable type in the name. Type may change!
    bool        g_abbrev_are_ok_if_obvious;     // abbreviations are ok, if they are obvious.
    
    // real examples from source:
    QString     m_character_description;
    bool        m_is_flying;
    uint32_t    m_experience_points;
    glm::vec3   m_pos;                          // with members m_pos.x, m_pos.y, m_pos.z;
    float       yaw;
```

6. Doxygen Comments - IMHO The place for doxygen comments is in the source file.
Headers should be clean of comments, as to allow for quick class interface scanning


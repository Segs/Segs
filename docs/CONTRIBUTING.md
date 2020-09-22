# Contributing to SEGS

:+1::tada: Thank you for contributing to the SEGS Project! :tada::+1:

### Regarding the leaked source code:
* Please be aware that SEGS is a [Clean Room](https://en.wikipedia.org/wiki/Clean_room_design) environment. 
* We cannot keep our project at it's current legal status if we were to use leaked source code directly.
 
In short, everything made for SEGS is original and entirely independent from the leaked code.

The following is a set of guidelines for contributing to SEGS. These are mostly guidelines, not rules. Use your best judgment, and feel free to propose changes to this document in a pull request.

#### Table Of Contents

[Code of Conduct](#code-of-conduct)

[I don't want to read this whole thing, I just have a question!!!](#i-dont-want-to-read-this-whole-thing-i-just-have-a-question)
  * [Chat with us on Discord](#chat-with-us-on-discord)

[What should I know before I get started?](#what-should-i-know-before-i-get-started)
  * [SEGS and Packages](#segs-and-packages)

[How Can I Contribute?](#how-can-i-contribute)
  * [Reporting Bugs or Enhancements or Feature Requests](#reporting-bugs-or-enhancements-or-feature-requests)
  * [Your First Code Contribution](#your-first-code-contribution)
  * [Pull Requests](#pull-requests)

[Styleguides](#styleguides)
  * [Git Commit Messages](#git-commit-messages)
  * [Coding Styleguide](#coding-styleguide)
  * [Documentation Styleguide](#documentation-styleguide)

[Additional Notes](#additional-notes)
  * [Issue and Pull Request Labels](#issue-and-pull-request-labels)

## Code of Conduct

This project and everyone participating in it is governed by the [SEGS Code of Conduct](CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please report unacceptable behavior to [segs@github.com](mailto:segs@github.com).

## I don't want to read this whole thing I just have a question!!!

> **Note:** _Please don't file an issue to ask a question._ You'll get faster results by reaching out to us on the SEGS Community Discord

### Chat with us on Discord

Come and join us in `#pocket-d` channel for general chat or `#troubleshooting` for support. 
Developers are welcome to join `#development`.  
 
 [**Click here to join SEGS Discord**](https://discord.segs.dev/)


## What should I know before I get started?

### SEGS and Packages

SEGS is a large open source project with ambitious goals and a complex code base. Before contributing, please read through the entirety of this guide, as it contains important information that will help ensure that your contributions are accepted and successful.


## How Can I Contribute?

### Reporting Bugs or Enhancements or Feature Requests

Bugs, Enhancements, and Feature Requests are tracked as [GitHub issues](https://guides.github.com/features/issues/).

This section guides you through submitting an issue for SEGS. Following these guidelines helps maintainers and the community understand your report :pencil:, reproduce the behavior :computer:, and find related reports :mag_right:.

Before creating issues, please check [this list](#before-submitting-an-issue) as you might find out that you don't need to create one. When you are creating an issue, please [include as many details as possible](#how-do-i-submit-a-good-issue). Fill out [the required template](ISSUE_TEMPLATE.md), the information it asks for helps us resolve issues faster.

> **Note:** If you find a **Closed** issue that seems like it is the same thing that you're experiencing, open a new issue and include a link to the original issue in the body of your new one.

#### Before Submitting An Issue

* **Check the [debugging guide](DEBUGGING_GUIDE.md).** You might be able to find the cause of the problem and fix things yourself. Most importantly, check if you can reproduce the problem [in the latest version of SEGS](http://github.com/Segs/Segs).
* **Check the [FAQs on the wiki](https://github.com/Segs/Segs/wiki/FAQ)** for a list of common questions and problems.
* **Perform a [cursory search](https://github.com/Segs/Segs/issues?q=is%3Aopen+is%3Aissue)** to see if the problem has already been reported. If it has **and the issue is still open**, add a comment to the existing issue instead of opening a new one.

#### How Do I Submit A (Good) Issue?

Bugs, Enhancements, and Feature Requests are tracked as [GitHub issues](https://guides.github.com/features/issues/). When creating an issue, please provide the following information by filling in [the template](ISSUE_TEMPLATE.md).

Explain the problem or feature and include additional details to help maintainers reproduce the problem:

* **Use a clear and descriptive title** for the issue to identify the problem.
* **Describe the exact steps which reproduce the problem** in as many details as possible. For example, start by explaining how you started SEGS, e.g. which command exactly you used in the terminal, or how you started SEGS otherwise. When listing steps, **don't just say what you did, but explain how you did it**. For example, if you moved your character, explain if you used the mouse, or a keyboard to move, and if you used any powers or slash commands.
* **Provide specific examples to demonstrate the steps**. Include links to files or GitHub projects, or copy/pasteable snippets, which you use in those examples. If you're providing snippets in the issue, use [Markdown code blocks](https://help.github.com/articles/markdown-basics/#multiple-lines).
* **Describe the behavior you observed after following the steps** and point out what exactly is the problem with that behavior.
* **Explain which behavior you expected to see instead and why.**
* **Include screenshots and animated GIFs** which show you following the described steps and clearly demonstrate the problem. If you use the keyboard while following the steps, **record the GIF with the [Keybinding Resolver](https://github.com/atom/keybinding-resolver) shown**. You can use [this tool](http://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and [this tool](https://github.com/colinkeenan/silentcast) or [this tool](https://github.com/GNOME/byzanz) on Linux.
* **If you're reporting that SEGS crashed**, include a crash report with a stack trace from the operating system, and the `crash.log` and `output.log` files from the SEGS directory. Include these reports in the issue in a [code block](https://help.github.com/articles/markdown-basics/#multiple-lines), a [file attachment](https://help.github.com/articles/file-attachments-on-issues-and-pull-requests/), or put it in a [gist](https://gist.github.com/) and provide link to that gist.
* **If the problem is related to performance or memory**, include a screenshot of your operating system's system monitor with your report, along with the `crash.log` and `output.log`.
* **If the problem wasn't triggered by a specific action**, describe what you were doing before the problem happened and share more information using the guidelines below.
* **Explain why this enhancement would be useful** to most SEGS users.
* **Cite a source demonstrating this behavior in the original CoX Servers** This is critical for showing intended behavior or output. Popular sources are https://ParagonWiki.com and https://youtube.com.

Provide more context by answering these questions:

* **Can you reproduce the problem after restarting the server and client?**
* **Did the problem start happening recently** (e.g. after updating to a new version of SEGS) or was this always a problem?
* If the problem started happening recently, **can you reproduce the problem in an older version of SEGS?** What's the most recent version in which the problem doesn't happen? You can download older versions of SEGS from [the releases page](https://github.com/Segs/Segs/releases).
* **Can you reliably reproduce the issue?** If not, provide details about how often the problem happens and under which conditions it normally happens.
* If the problem is related to working with files (e.g. opening and editing files), **does the problem happen for all files and projects or only some?** Does the problem happen only when working with local or remote files (e.g. on network drives), with files of a specific type (e.g. only JavaScript or Python files), with large files or files with very long lines, or with files in a specific encoding? Is there anything else special about the files you are using?

Include details about your configuration and environment:

* **Which version of SEGS are you using?** You can get the exact version by running `authserver -version` in your terminal.
* **What's the name and version of the OS you're using**?
* **Are you running SEGS in a virtual machine or using WINE?** If so, which VM software are you using and which operating systems and versions are used for the host and the guest?


### Your First Code Contribution

Unsure where to begin contributing to SEGS? You can start by looking through these `good-first-issue` and `wishlist` issues:

* [Good First Issue](https://github.com/Segs/Segs/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22) - issues which should only require a small number of lines of code, and a test or two.
* [Wishlist issues](https://github.com/Segs/Segs/issues?q=is%3Aopen+is%3Aissue+label%3Awishlist) - issues which should be a bit more involved than `good-first-issue` issues. These are issues that the SEGS core development team would love to implement or fix, but can't prioritize currently.

Both issue lists are sorted by total number of comments. While not perfect, number of comments is a reasonable proxy for impact a given change will have.

If you want to read about running SEGS, try the end-user [README](../Projects/CoX/docs/README.md).

#### Local development

SEGS can be developed locally by cloning the git repo to your local machine. For instructions on how to do this, see the following sections in the development [README](../README.md):


### Pull Requests

* Fill in [the required template](PULL_REQUEST_TEMPLATE.md)
* Do not include issue numbers in the PR title
* Include screenshots and animated GIFs in your pull request whenever possible.
* Include thoughtfully-worded, well-structured commit messages. See the [Coding Styleguide](#coding-styleguide) below.
* Document new code based on the [Documentation Styleguide](#documentation-styleguide)
* End all files with a newline


## Styleguides

### Git Commit Messages

* Use the present tense ("Add feature" not "Added feature")
* Use the imperative mood ("Move cursor to..." not "Moves cursor to...")
* Limit the first line to 72 characters or less
* Reference issues and pull requests liberally after the first line
* When only changing documentation, include `[ci skip]` in the commit description
* If creating a new file, please add the standard SEGS copyright notice to the head of the file
* Consider starting the commit message with an applicable emoji:
    * :art: `:art:` when improving the format/structure of the code
    * :racehorse: `:racehorse:` when improving performance
    * :non-potable_water: `:non-potable_water:` when plugging memory leaks
    * :memo: `:memo:` when writing docs
    * :card_file_box: `:card_index:` when modifying the databases
    * :scroll: `:scroll:` when modifying the LUA Scripts
    * :penguin: `:penguin:` when fixing something on Linux
    * :apple: `:apple:` when fixing something on macOS
    * :checkered_flag: `:checkered_flag:` when fixing something on Windows
    * :bug: `:bug:` when fixing a bug
    * :fire: `:fire:` when removing code or files
    * :green_heart: `:green_heart:` when fixing the CI build
    * :white_check_mark: `:white_check_mark:` when adding tests
    * :lock: `:lock:` when dealing with security
    * :arrow_up: `:arrow_up:` when upgrading dependencies
    * :arrow_down: `:arrow_down:` when downgrading dependencies

### Coding Styleguide

1. Use spaces instead of tabs. ( Most editors allow one to set this up so 'tab' = 4 spaces )

2. The target brace style is: 
```c++
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
```c++
    if (very very very very very very very very very very very very
            very very long predicate)
    {
        statement; ...
```

4. In header files, reserve 2 tabs for the optional keywords virtual or static,
and reserve enough tabs for the longest return type. i.e:
```c++
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

6. Use spacing between binary mathematical, assignment, and ternary operators and their operands:
```cpp
    a = b + c;
    a += x * y / z;
    a = b ? c : d;
```

7. Doxygen documentation should be done in the implementation files (.cpp) and omitted from the headers. Proper and thorough documentation will enable others to better understand the intent of any code you contributed. Doxygen uses special comment syntax along with tags to accomplish this. For a definitive list of tags, please see [Doxygen tags](https://www.stack.nl/~dimitri/doxygen/manual/commands.html). We prefer Qt style (`/*!*/` for multi-line and `//!` for single-line) comments. See below for various examples.
```cpp
//! This class represents an Entity
class Entity
{
    public:
        // ...
    private:
        int id; //! contains the ID of the Entity
};

/*!
 * @brief Clones an entity
 * @param[in] src source Entity to copy from
 * @param[out] dst destination Entity to copy to
 * @returns true if copy was successful and false otherwise
 */
bool clone(Entity *src, Entity *dst)
{
    // ...
    if (!Entity)
        return false;
    else
        return true;
}
```

### Documentation Styleguide

* Use [Markdown](https://daringfireball.net/projects/markdown) whenever possible.


## Additional Notes

### Issue and Pull Request Labels

This section lists the labels we use to help us track and manage issues and pull requests. Open an issue to suggest new labels.

[GitHub search](https://help.github.com/articles/searching-issues/) makes it easy to use labels for finding groups of issues or pull requests you're interested in. For example, you might be interested in [open issues which are labeled as bugs, but still need to be reliably reproduced](https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Aissue+user%3ASegs+label%3Abug+label%3Aneeds-reproduction) or perhaps [open pull requests which haven't been reviewed yet](https://github.com/issues?utf8=%E2%9C%93&q=is%3Aopen+is%3Apr+repo%3ASegs%2Fatom+comments%3A0). To help you find issues and pull requests, each label is listed with search links for finding open items with that label. We  encourage you to read about [other search filters](https://help.github.com/articles/searching-issues/) which will help you write more focused queries.

The labels are loosely grouped by their purpose, but it's not required that every issue have a label from every group or that an issue can't have more than one label from the same group.

Please open an issue on `Segs/Segs` if you have suggestions for new labels.

#### Type of Issue and Issue State

| Label Name | Search :mag_right: | Description |
| --- | --- | --- |
| `bug` | [search][search-label-bug] | Confirmed bugs or reports that are very likely to be bugs. |
| `enhancement` | [search][search-label-enhancement] | Feature requests. |

#### Topic Categories

| Label Name | Search :mag_right: | Description |
| --- | --- | --- |
| `good-first-issue` | [search][search-label-good-first-issue] | Issues that are great for beginners, or those new to the SEGS project. |
| `wishlist` | [search][search-label-wishlist] | Issues that the SEGS core development team would love to implement or fix, but can't prioritize currently. |
| `network-layer-stability` | [search][search-label-network-layer-stability] | Related to SEGS network layer. |

#### Pull Request Labels

| Label Name | Search :mag_right: | Description |
| --- | --- | --- |
| `work-in-progress` | [search][search-label-work-in-progress] | Pull requests which are still being worked on, more changes will follow. |
| `needs-review` | [search][search-label-needs-review] | Pull requests which need code review, and approval from maintainers or SEGS core team. |
| `under-review` | [search][search-label-under-review] | Pull requests being reviewed by maintainers or SEGS core team. |
| `requires-changes` | [search][search-label-requires-changes] | Pull requests which need to be updated based on review comments and then reviewed again. |
| `needs-testing` | [search][search-label-needs-testing] | Pull requests which need manual testing. |

[search-label-bug]: https://github.com/Segs/Segs/issues?q=is%3Aopen+is%3Aissue+label%3Abug
[search-label-enhancement]: https://github.com/Segs/Segs/issues?q=is%3Aopen+is%3Aissue+label%3Aenhancement

[search-label-good-first-issue]: https://github.com/Segs/Segs/issues?q=is%3Aopen+is%3Aissue+label%3Agood-first-issue
[search-label-wishlist]: https://github.com/Segs/Segs/issues?q=is%3Aopen+is%3Aissue+label%3Awishlist
[search-label-network-layer-stability]: https://github.com/Segs/Segs/issues?q=is%3Aopen+is%3Aissue+label%3A%22network+layer+stability%22

[search-label-work-in-progress]: https://github.com/Segs/Segs/pulls?q=is%3Aopen+is%3Apr+label%3Awork-in-progress
[search-label-needs-review]: https://github.com/Segs/Segs/pulls?q=is%3Aopen+is%3Apr+label%3Aneeds-review
[search-label-under-review]: https://github.com/Segs/Segs/pulls?q=is%3Aopen+is%3Apr+label%3Aunder-review
[search-label-requires-changes]: https://github.com/Segs/Segs/pulls?q=is%3Aopen+is%3Apr+label%3Arequires-changes
[search-label-needs-testing]: https://github.com/Segs/Segs/pulls?q=is%3Aopen+is%3Apr+label%3Aneeds-testing

This documentation was built off of the amazing work at https://github.com/atom/atom/blob/master/CONTRIBUTING.md

## Thank you and happy coding!

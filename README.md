
[top_level]

1 -> int

[top_level, name]

2 -> x

[top_level, name, name] = [top_level, var_decl]

3 -> ;

[top_level, var_decl, ;] = [top_level, declaration] = [top_level]

4 -> int 

[top_level, name]

5 -> main

[top_level, name, name] = [top_level, var_decl]

6 -> (

[top_level, var_decl, ( ] = [top_level, func_decl]

7 -> char 

[top_level, func_decl, name]

8 -> *

[top_level, func_decl, name, asterisk] = [top_level, func_decl, name_with_asterisk]

9 -> *

[top_level, func_decl, name_with_asterisk, asterisk] = [top_level, func_decl, name_with_multiple_asterisk]

10 -> argv

[top_level, func_decl, name_with_multiple_asterisk, name] = [top_level, func_decl, var_decl]

11 -> +



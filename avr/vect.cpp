
char foo(char c)
{
   char v1ui __attribute(( vector_size(1) ));
   v1ui[0] = c;
   v1ui >>= 1;
   return v1ui[0];
}


{
  if( /#line/ && $1 != ($3 - 1)) {
    printf "Line directive mismatch at line %d: %s\n", NR, $0;
    exit 1;
  }
}


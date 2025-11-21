ENTRY(_start)
SECTIONS
{
  . = 1M;
  .text :
  {
    *(.text)
  }
  _kend = .;
}

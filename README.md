# CSPLIT.C
![image of csplit running](https://github.com/jthro/csplit/blob/main/csplit.png)

A simple terminal split timer written in C.

Motivation: I was going to do an exam practice and realized I didn't have a nice split timer, and writing one would probably be good practice.
Note: Will not count more than 63 splits or 1.2 million hours

## Building csplit
| compiler | command |
|----------|---------|
| clang    | clang csplit.c -o csplit |
| gcc      | gcc csplit.c -o csplit |

## Usage
| command | usage |
|---------|--------|
| csplit --help    | list keybinds and flags |
| q      | quit csplit |
| s      | new split |

### TODO:
- [ ] Add custom time formats
- [ ] Add user-defined refresh rate
- [ ] Add user-defined colors
- [ ] Make it look prettier
- [ ] Long term: support config files

# c-puf

A portable, event driven library, pub/sub with on/emit methods, written in C,
object oriented. It allows to trigger events with associated callbacks.

## Example

Start by creating an object that will hold the events and callbacks:

```c
// event definitions
#define BUTTON_1_CLICKED 0x01
#define BUTTON_2_CLICKED 0x02

// the events object
puf *ev;

int main() {
  // Create two events with a limit of ten callbacks (each)
  ev = puf_new(2, 10);
}
```

Add some events with `ev->on()`

```c
[...]

int cbBtn1(uint8_t *param);
int cbBtn2(uint8_t *param);

int main() {
  ev = puf_new(2, 10);
  ev->on(ev, BUTTON_1_CLICKED, cbBtn1);
  ev->on(ev, BUTTON_2_CLICKED, cbBtn2);
}

int cbBtn1(uint8_t *param) {
  // this callback does not accept NULL (because no reason, because I can)
  if (param == NULL) {
    printf("ERROR: No param, I need a param\n");
    // An error code. Will interrupt the execution of other callbacks
    // (on this example there is only one callback for this event)
    return 1;
  }

  char value[256] = {0}; // this is not secure, but this is an example ;)
  // macro to convert the uint8 param in a char
  PUF_READ_CHAR(value, param, 255)
  // print the param value
  printf("Button 1 called with param: %s.\n", value);
  // remember always to return an error code
  return 0; // ok result, no error
}

int cbBtn2(uint8_t *param) {
  (void) param; // this callback does not require a param
  printf("Button 2 called.\n");
  // remember always to return an error code
  return 0; // ok result, no error
}
```

Trigger the event with `ev->emit()`

```C
[...]

int main() {
  ev = puf_new(2, 10);
  ev->on(ev, BUTTON_1_CLICKED, cbBtn1);
  ev->on(ev, BUTTON_2_CLICKED, cbBtn2);

  // Trigger the event
  // This lines should be in another part of the project, not the same place
  // where the listener is created (it wouldn't make much sense).
  // Again it is an example ;)
  char param[] = "Hello World";
  int err = ev->emit(ev, BUTTON_1_CLICKED, PUF_ARG(param));
  if (err != 0) printf("ERROR Button 1 %d\n", err);

  int err = ev->emit(ev, BUTTON_2_CLICKED, PUF_ARG(param));
  if (err != 0) printf("ERROR Button 2 %d\n", err);
}

[...]
```

## Object oriented

This is a personal project where I explore the use of object oriented
programming with Ansi-C. Previously I created a similar library with C++, Puf.

The absence of the `this` keyword, in C, leads you to do things like this:

```c
int err = ev->emit(ev, BUTTON_1_CLICKED, PUF_ARG_P(param));
```

Where the first parameter of the function is the object of the function itself,
thus replacing the need for the `this` object. This adds a bit of redundancy in
the code, but hey, we are talking about C, nobody ever said things were going to
be easy :)

## NO-RTOS

C-Puf is simple and lightweight and has no dependencies. This makes it suitable
for use in embedded systems with few resources, especially if you do not use an
RTOS. In RTOS there are other better options, e.g. message queues.

[Puf][1] was used in production environments with multiple IoT/NO-RTOS applications.
Devices that have been running for years without apparent errors, with a lot of
heavy workload (constant connections and sending data to servers).

I want to believe that C-Puf could do it too ;)

## Passing parameters

> Although in the examples an array of chars is used, it is not limited to this,
> other types of data can also be used, such as structs, int... etc.

One of the usual needs in a pub/sub environment is that callbacks can receive
one or more parameters.

Previously, in [Puf][1], callbacks could only receive a parameter of type `int`,
which limited their use. In C-Puf I have preferred to use a uint8* data type,
which allows them to be casted easily.

In C the usual way for a function to receive multiple parameters is to use the
`stdarg.c` library, however I did not want to use it to avoid adding dependencies
to the project.

```c
#define PARAM_SIZE 15

// the original string
char str[PARAM_SIZE + 1] = "Hello World";

// conversion
uint8_t *str_u = (uint8_t*) &str;

// pass the uint8 to a function
myFunction(str_u);

// in myFunction, restore the original string with memcpy
void myFunction(uint8_t * str_u) {
  char param[PARAM_SIZE + 1] = {'\0'}; // Important to initialize the array
                                       // add an extra char for the last null.
  memcpy(&param, str_u, PARAM_SIZE);   // Now "param" is "Hello World" (original str)
}
```

### Another way to pass parameters: void

```C
#define PARAM_SIZE 15

char str[PARAM_SIZE + 1] = "Hello World";

void myFunction(void * param) {
    char *value = (char*) param;
    printf("%s\n", value);
}

// call the function
myFunction(str);
```

## Macros to convert and restore from/to uint8_t

Use this macros to convert to uint8_t

```c
#define PUF_ARG(str) (uint8_t*) &str
#define PUF_ARG_P(str) (uint8_t*) str

// use PUF_ARG (for char or any other data type)
char param[] = "Hello World";
int err = ev->emit(ev, BUTTON_1_CLICKED, PUF_ARG(param));
                                      // (uint8_t*) &param

// use PUF_ARG_P (for pointers)
char * param = "Hello World";
int err = ev->emit(ev, BUTTON_1_CLICKED, PUF_ARG_P(param));
```

[1]: https://github.com/jenguidanos/puf

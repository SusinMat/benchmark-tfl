beeswax by LEDL inspired by TensorFlow Lite's label_image.

o run it. Prepare `./mobilenet_quant_v1_224.tflite`, `./grace_hopper.bmp`, and `./labels.txt`.

Run it:
```
> ./beeswax                                        
Loaded model ./mobilenet_quant_v1_224.tflite
resolved reporter
invoked
average time: 100.986 ms 
0.439216: 653 military uniform
0.372549: 458 bow tie
0.0705882: 466 bulletproof vest
0.0235294: 514 cornet
0.0196078: 835 suit
```
Run `interpreter->Invoker()` 100 times:
```
> ./beeswax   -c 100                               
Loaded model ./mobilenet_quant_v1_224.tflite
resolved reporter
invoked
average time: 33.4694 ms
...
```

Run a floating point (`mobilenet_v1_1.0_224.tflite`) model,
```
> ./beeswax -f 1 -m mobilenet_v1_1.0_224.tflite
Loaded model mobilenet_v1_1.0_224.tflite
resolved reporter
invoked
average time: 263.493 ms 
0.88615: 653 military uniform
0.0422316: 440 bearskin
0.0109948: 466 bulletproof vest
0.0105327: 401 academic gown
0.00947104: 723 ping-pong bal
```

See the source code for other command line options.

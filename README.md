## build python module
```
apt-get install libfaad-dev libfaad2
```
```
python3 setup.py build
```
## test library
```
ln -sv build/lib.linux-x86_64-3.6/*.so faad2.so
python3 -c "import faad2; dec = faad2.Faad2Dec(); conf=dec.GetCurrentConfiguration(); print(conf); dec.SetConfiguration(**conf)"
>>> {'defObjectType': 1, 'defSampleRate': 44100, 'outputFormat': 1, 'downMatrix': 0, 'useOldADTSFormat': 0}
```

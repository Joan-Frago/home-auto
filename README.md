# SYSFS
## DI STATUS READING

```bash
cat /run/unipi-plc/by-sys/DI1.1/value
```

Status reading of all first section DI's:

```bash
cat /run/unipi-plc/by-sys/DI[1-3].[0-9][0-9]/value
```

Status reading of all available DI units:

```bash
cat /run/unipi-plc/by-sys/DI[1-3].[0-9][0-9]/value
```

# @author Peter Bigot

# FOR THE DEVELOPER:
# Customize your agent by giving the name of the thing you will be controlling
# No spaces! These will go into variable and function names.
# Three spellings: all caps, intercapped, all lower
# Change "Gadget" into "MyDeviceName", whatever your device name is.

SED_ARGS='-e s@GADGET@MYDEVICENAME@g -e s@Gadget@MyDeviceName@g -e s@gadget@mydevicename@g'

# That's it.  The next two commands take care of the rest for you.

# Fix file contents
find . -path ./.git -prune -o -type f -print0 \
  | xargs -0 grep -il gadget \
  | xargs sed -i ${SED_ARGS}

# Fix file names
for fn in $(find . -path ./.git -prune -o -iname '*gadget*' -print) ; do
  nfn=$(echo ${fn} | sed ${SED_ARGS})
  git mv ${fn} ${nfn}
done

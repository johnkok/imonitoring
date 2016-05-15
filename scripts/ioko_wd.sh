#! /bin/bash
#
#


case "$(pidof ioko | wc -w)" in

0)  echo "Restarting ioko:"
    sudo /var/www/ioko/./ioko
    ;;
1)  echo "ioko already running"
    ;;
esac

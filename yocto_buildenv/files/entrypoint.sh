#!/bin/bash

#blatantly stolen from https://github.com/rocker-org/rocker

## Don't attempt to run if we are not root
if [ "$EUID" -ne 0 ]
    then echo "Please run as root"
    exit
fi

if [ "$USER" = root ]
    then unset USER
fi

## Set defaults for environmental variables in case they are undefined
USER=${USER:=build}
PASSWORD=${PASSWORD:=build}
EMAIL=${EMAIL:=build@example.com}
USERID=${USERID:=1000}
ROOT=${ROOT:=FALSE}

#check if the user doesn't exist
if ! id -u $USER > /dev/null 2>&1; then
    ## Things get messy if we have more than one user.
    ## (Docker cares only about uid, not username; diff users with same uid = confusion)
    if [ "$USERID" -ne 1000 ]
    ## Configure user with a different USERID if requested.
        then
            echo "creating new $USER with UID $USERID"
            useradd -m $USER -u $USERID
            mkdir /home/$USER
            chown -R $USER /home/$USER
    else
        ## RENAME the existing user. (because deleting a user can be trouble, i.e. if we're logged in as that user)
        usermod -l $USER build
        usermod -m -d /home/$USER $USER 
        groupmod -n $USER build 
        echo "USER is now $USER"
    fi
    ## Assing password to user
    echo "$USER:$PASSWORD" | chpasswd

    ## Configure git for the User. Since root is running this script, cannot use `git config`
    echo -e "[user]\n\tname = $USER\n\temail = $EMAIL\n\n[credential]\n\thelper = cache\n\n[push]\n\tdefault = simple\n\n[core]\n\teditor = vim\n" > /home/$USER/.gitconfig
    echo ".gitconfig written for $USER"

    addgroup $USER staff

    # Use Env flag to know if user should be added to sudoers
    if [ "$ROOT" == "TRUE" ]
        then
            adduser $USER sudo && echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers
            echo "$USER added to sudoers"
    fi

    ## User should own their own home directory and all containing files (including these templates)
    chown -R $USER /home/$USER
fi

#spawn a shell or whatever else is desired
su - $USER


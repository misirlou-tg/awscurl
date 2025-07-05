# awscurl

Curl functionality with AWS V4 signatures with support for AWS profiles/sso

## Usage

```
awscurl [OPTIONS] url

POSITIONALS:
  url (required)              URL to make request to

OPTIONS:
  -h,     --help              Print this help message and exit
  -X,     --request <verb>    GET, POST, DELETE, PUT, HEAD, PATCH
                              Specify request method to use (default: GET)
  -d,     --data <data>       Data to POST, @file to read from 'file'
          --data-binary       Read POST data file as binary, keep line breaks
  -H,     --header <header> ...
                              Request header to add ("<header>: <value>")
  -i,     --include           Include response headers in output
  -k,     --insecure          Allow insecure server connections when using https
          --profile <profile> AWS profile
          --region <region>   AWS region (default: profile from client config)
          --service <service> AWS service (default: "execute-api")
```

Loading of credentials is much like the `aws` CLI, a profile can be specified
with `--profile` or the [Default credential provider chain][cred-providers] is
used.

[cred-providers]: https://sdk.amazonaws.com/cpp/api/LATEST/root/html/md_docs_2_credentials___providers.html

### Running via Docker
```
# Allow access to local AWS config/credentials
docker run -it --rm -v "$HOME/.aws:/home/ubuntu/.aws" misirloutg/awscurl <arguments>
```

To pass AWS standard environment variables
```
docker run -it --rm \
  -e AWS_ACCESS_KEY_ID \
  -e AWS_SECRET_ACCESS_KEY \
  -e AWS_SECURITY_TOKEN \
  -e AWS_PROFILE \
  misirloutg/awscurl <arguments>
```

Open a shell
```
docker run -it --rm -v "$HOME/.aws:/home/ubuntu/.aws" --entrypoint /bin/bash misirloutg/awscurl
```

## Building

This uses packages from [vcpkg][vcpkg] ([GitHub][vcpkg-github]) in "manifest mode"

[vcpkg]: https://vcpkg.io/
[vcpkg-github]: https://github.com/microsoft/vcpkg

vcpkg references:
- https://learn.microsoft.com/en-us/vcpkg/consume/manifest-mode
- https://learn.microsoft.com/en-us/vcpkg/users/buildsystems/cmake-integration

### Windows

There is a `.sln` and `.vcxproj` to build on Windows with Visual Studio, has
been tested on Visual Studio 2022

### Linux

For Linux there is a `CMakeLists.txt`, has been tested on Ubuntu 24.04

Pre-requisites, see vcpkg [Geting Started][vcpkg-getting-started]
- Compiler (Ubuntu `build-essential`) and CMake are installed
- vcpkg is cloned in the root of the repo in sub-dir `vcpkg`
- vcpkg has been bootstrapped

Configure
```
cmake -B build -S .
```

Build `awscurl`
```
cmake --build build
```

Build docker image
```
docker build --tag awscurl:latest .
```

[vcpkg-getting-started]: https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash

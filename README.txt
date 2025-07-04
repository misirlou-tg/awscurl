# AWS Service endpoints:
# https://docs.aws.amazon.com/general/latest/gr/rande.html

# S3 URL is https://[<bucketName>.]s3[-<region>].amazonaws.com/
# Where region is omitted for us-east-1
# Where bucketName is only needed for bucket operations
# (found in sample here: https://docs.aws.amazon.com/AmazonS3/latest/API/sig-v4-examples-using-sdks.html#sig-v4-examples-using-sdk-dotnet)
#
# List buckets
awscurl --service s3 https://s3.amazonaws.com/

# EC2 URL is https://ec2[.region].amazonaws.com/
# Where region is omitted for us-east-1
# (see https://docs.aws.amazon.com/AWSEC2/latest/APIReference/Using_Endpoints.html)
#
# Describe instances
# https://docs.aws.amazon.com/AWSEC2/latest/APIReference/API_DescribeInstances.html
# The above doc does not say so, but for the filter syntax to work you have to specify a more recent version
awscurl --service ec2 https://ec2.amazonaws.com/?Action=DescribeInstances
awscurl --service ec2 "https://ec2.amazonaws.com/?Action=DescribeInstances&Version=2016-11-15&Filter.1.Name=tag:Name&Filter.1.Value.1=gemini-one"
awscurl --service ec2 "https://ec2.amazonaws.com/?Action=DescribeInstances&Version=2016-11-15&Filter.1.Name=instance-id&Filter.1.Value.1=i-XXXX"
# Start/stop instances (more recent version required!)
awscurl --service ec2 "https://ec2.amazonaws.com/?Action=StartInstances&Version=2016-11-15&InstanceId.1=i-XXXX"
awscurl --service ec2 "https://ec2.amazonaws.com/?Action=StopInstances&Version=2016-11-15&InstanceId.1=i-XXXX"

# CloudFormation URL is https://cloudformation.<region>.amazonaws.com
# Note: region is required, even for us-east-1
#
# Describe stacks
# (I chose XML output because it is "pretty printed")
awscurl --service cloudformation --header "accept: application/xml" https://cloudformation.us-east-1.amazonaws.com/?Action=DescribeStacks

# SSM URL is is https://ssm.<region>.amazonaws.com/
# https://docs.aws.amazon.com/general/latest/gr/ssm.html
# (Found in AWS Services endpoints link at top of README)
#
# Describe SSM parameters
# https://docs.aws.amazon.com/systems-manager/latest/APIReference/API_DescribeParameters.html
awscurl --service ssm --header "Accept: application/json" --header "X-Amz-Target: AmazonSSM.DescribeParameters" --header "Content-Type: application/x-amz-json-1.1" --data "{ }" https://ssm.us-east-1.amazonaws.com

# Curl supports adding an AWS V4 signature:
# https://curl.se/docs/manpage.html#--aws-sigv4
#
# This is workable for testing, etc, but is much less flexible than using --profile
# and using SSO or where a profile is using an assumed role.
#
# For example the List buckets & Describe instances requests from above would look like:
# (have to replace region, key [id] and secret from .aws config / credentials file)
curl --aws-sigv4 "aws:amz:region:s3" --user "key:secret" https://s3.amazonaws.com/
curl --aws-sigv4 "aws:amz:region:ec2" --user "key:secret" https://ec2.amazonaws.com/?Action=DescribeInstances

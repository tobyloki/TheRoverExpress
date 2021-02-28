const AWS = require('aws-sdk');
AWS.config.region = 'us-east-1';
const kinesis = new AWS.Kinesis();

const streamName = 'RoverStream';
let currentShardIterator;

exports.main = async (cb) => {
    const shardId = await getShardId();
    currentShardIterator = await getShardIterator(shardId);

    pollRecords();
    setInterval(async () => {
        pollRecords(cb);
    }, 2000);
}

async function pollRecords(cb){
    // console.log('Fetching latest records...');
    const records = await getRecords(currentShardIterator);    
    for(record of records){
        const data = JSON.parse(record.Data.toString());
        // console.log(data);
        cb(data);
    }
}

async function getRecords(shardIterator){
    return new Promise((resolve, reject) => {
        const params = {
            ShardIterator: shardIterator
        };
        kinesis.getRecords(params, (err, data) => {
            if(err){
                console.log(err);
                reject(err);
            } else {
                const {Records, NextShardIterator} = data
                currentShardIterator = NextShardIterator;
                // console.log('Records received: ' + Records.length);
                // console.log(data)
                resolve(Records);
            }
        });
    });
}

async function getShardIterator(shardId){
    return new Promise((resolve, reject) => {
        const params = {
            ShardId: shardId,
            ShardIteratorType: 'LATEST',
            StreamName: streamName
        };
        kinesis.getShardIterator(params, (err, data) => {
            if(err){
                console.log(err);
                reject(err);
            } else {
                const {ShardIterator} = data;
                // console.log('ShardIterator: ' + ShardIterator);
                resolve(ShardIterator);
            }
        });
    });
}

async function getShardId(){
    return new Promise((resolve, reject) => {
        const params = {
            StreamName: streamName
        };
        kinesis.describeStream(params, (err, data) => {
            if(err){
                console.log(err);
                reject(err);
            } else {
                const {ShardId} = data.StreamDescription.Shards[0]
                // console.log('ShardId: ' + ShardId);
                resolve(ShardId);
            }
        });
    });
}

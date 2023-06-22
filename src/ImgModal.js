import React from 'react';
import { Button, Modal, ModalHeader, ModalBody, ModalFooter } from 'reactstrap';
import 'bootstrap/dist/css/bootstrap.css';
//import OpenModal from './OpenModalContext';

const ImgModal = (props) => {
    const {
        CID,
        show,
        onClose
    } = props;
    return (
        <Modal isOpen={show} fade={true} toggle={show}>
            <ModalHeader toggle={onClose}>View Image</ModalHeader>
            <ModalBody>
                {CID}-CID가 잘 전달됨
            </ModalBody>
            <ModalFooter>
                <Button color='primary' onClick={onClose}>확인</Button>
            </ModalFooter>
        </Modal> 
    );
};

export default ImgModal;